#include "Client.h"

#include <iostream>

#include "Core/Log.h"

#define MAX_NETWORK_READ_RETRIES 200

Client::Client(const ClientConfig &config)
	: m_Config(config), m_Camera(false, 1280, 720)
{
	std::string cwd = "";
	Core::FileSystem::Get()->GetCurrentWorkingDirectory(&cwd);
	m_Version = Core::utils::GetLocalVersion("../../..");

	CAM_LOG_INFO("===================== CONFIG ===================================");
	CAM_LOG_INFO("IP                    : {}", config.ServerIP);
	CAM_LOG_INFO("Port                  : {}", config.Port);
	CAM_LOG_INFO("Current Client version: {}", m_Version);
	CAM_LOG_INFO("Current CWD           : {}", cwd);
	CAM_LOG_INFO("================================================================");

	m_Socket = Core::Socket::Create();
	if (!m_Socket->Open(true, m_Config.ServerIP, m_Config.Port))
	{
		CAM_LOG_ERROR("Socket could not be opened!");
	}
	if (!m_Socket->SetNonBlocking(true))
	{
		CAM_LOG_ERROR("Socket could not be set to non-blocking!");
	}

	m_Host = m_Socket->Lookup(m_Config.ServerIP, m_Config.Port);
}

Client::~Client()
{
	Release();
}

void Client::Release()
{
	CAM_LOG_INFO("Releasing all resources.");

	if (m_CameraThread.joinable())
		m_CameraThread.join();

	if (m_NetworkThread.joinable())
		m_NetworkThread.join();

	m_Camera.Release();

	if (m_Socket)
	{
		CAM_LOG_INFO("Releasing socket");
		delete m_Socket;
		m_Socket = nullptr;
	}
}

void Client::Run(bool shouldShowFrames)
{
	m_NetworkThread = std::thread(&Client::NetworkLoop, std::ref(*this));
	m_CameraThread = std::thread(&Client::CameraLoop, std::ref(*this));

	if (shouldShowFrames)
	{
		Show();
	}
	else
	{
		while (m_Running)
		{
		}
	}

	// wait until the network thread is finished
	while (!m_NetworkThreadFinished)
	{
	}
}

void Client::NetworkLoop()
{
	CAM_LOG_DEBUG("Sending connection start request to server...");
	ClientConnectionStartMessage msg = {};
	msg.Header.Type = CLIENT_CONNECTION_START;
	msg.Header.Version = m_Version;
	msg.FrameName = "Client #1";
	msg.FPS = 30;
	m_Socket->Send(&msg, sizeof(msg), m_Host);

	while (true)
	{
		static Byte BUF[65536];
		static uint32 current_connection_retry = 0;

		if (!m_Running && !m_SentConnectionCloseRequest)
		{
			// send request to server, that we want to close the connection
			CAM_LOG_DEBUG("Sending connection close request to server...");
			ClientConnectionCloseMessage close_msg = {};
			close_msg.Header.Type = CLIENT_CONNECTION_CLOSE;
			close_msg.Header.Version = m_Version;
			m_Socket->Send(&close_msg, sizeof(close_msg), m_Host);
			m_SentConnectionCloseRequest = true;
		}

		Core::addr_t addr;
		int32 len = m_Socket->Recv(BUF, sizeof(BUF), &addr);
		if (len < 0)
		{
			++current_connection_retry;
			if (current_connection_retry >= MAX_NETWORK_READ_RETRIES && !m_ConnectedToServer)
			{
				CAM_LOG_ERROR("Fatal error: Could not connect to server!");
				m_Running = false;
				m_NetworkThreadFinished = true;
				return;
			}
			else if (current_connection_retry >= 100 * MAX_NETWORK_READ_RETRIES && m_ConnectedToServer)
			{
				CAM_LOG_ERROR("Fatal error: Lost connection to server!");
				m_Running = false;
				m_NetworkThreadFinished = true;
				return;
			}

			CAM_LOG_ERROR("Failed to receive data from network layer!");
			continue;
		}
		else
		{
			current_connection_retry = 0;
		}

		// ignore all messages from unknown senders
		if (addr.Value != m_Host.Value)
		{
			CAM_LOG_ERROR("Unknown sender!");
			continue;
		}

		if (len < sizeof(header_t))
		{
			CAM_LOG_ERROR("Unexpected header size!");
			continue;
		}

		header_t *header = (header_t *)BUF;

		if (header->Version != m_Version)
		{
			CAM_LOG_ERROR("Unexpected version encountered.");
			continue;

		}

		bool message_success = false;
		switch (header->Type)
		{
			case SERVER_CONNECTION_START:
				message_success = OnConnectionAccepted(BUF, len);
				break;

			case SERVER_CONNECTION_CLOSE:
				message_success = OnConnectionClosed(BUF, len);
				break;

			case SERVER_FRAME:
				message_success = OnFrameResponse(BUF, len);
				break;
		}

		if (!message_success)
		{
			CAM_LOG_ERROR("Specific message handler failed. Aborting.");
			break;
		}

		// If we handled the server connecection close request successfully, quit from the network thread.
		if (header->Type == SERVER_CONNECTION_CLOSE && message_success)
		{
			m_NetworkThreadFinished = true;
			CAM_LOG_INFO("We got the connection close response from the server and it was successful. Closing Connection.");
			break;
		}
	}
}

void Client::CameraLoop()
{
	while (m_Running)
	{
		if (!m_Camera.IsRunning())
		{
			m_Running = false;
			break;
		}

		m_Camera.GenerateFrames();
	}
}

void Client::Show()
{
	while (m_Running)
	{
		if (!m_Camera.IsRunning())
		{
			m_Running = false;
			break;
		}

		uint32 frame_size = 0;
		uint32 frame_width = 0;
		uint32 frame_height = 0;
		Byte *frame = m_Camera.ShowLive(&frame_size, &frame_width, &frame_height);

		if (!frame)
		{
			CAM_LOG_ERROR("Could not read image from camera!");
			continue;
		}

		ProcessFrame(frame, frame_size, frame_width, frame_height);
		SendFrameToServer(frame, frame_size, frame_width, frame_height);

		delete[] frame;
	}
}

bool Client::OnConnectionAccepted(Byte *message, uint32 length)
{
	if (length != sizeof(ServerConnectionStartResponse))
	{
		CAM_LOG_ERROR("Unexpected message size encountered.");
		return false;
	}

	CAM_LOG_DEBUG("Trying to connect to server...");
	ServerConnectionStartResponse *msg = (ServerConnectionStartResponse *)message;

	if (!msg->ConnectionAccepted)
	{
		CAM_LOG_ERROR("Server responded unsuccessful to connection start request!");
		return false;
	}

	m_ConnectedToServer = true;
	CAM_LOG_INFO("Connected to server successfully!");
	return true;
}

bool Client::OnConnectionClosed(Byte *message, uint32 length)
{
	if (length != sizeof(ServerConnectionCloseResponse))
	{
		CAM_LOG_ERROR("Unexpected message size encountered.");
		return false;
	}

	CAM_LOG_DEBUG("Trying to disconnect from server...");
	ServerConnectionCloseResponse *msg = (ServerConnectionCloseResponse *)message;

	if (!msg->ConnectionClosed)
	{
		CAM_LOG_ERROR("Server responded unsuccessful to connection close request!");
		return false;
	}

	CAM_LOG_INFO("Disconnected from server successfully!");
	return true;
}

bool Client::OnFrameResponse(Byte *message, uint32 length)
{
	if (length != sizeof(ServerFrameResponse))
	{
		CAM_LOG_ERROR("Unexpected message size encountered.");
		return false;
	}

	std::cout << "Trying to read back the frame response from server..." << std::endl;
	ServerFrameResponse *msg = (ServerFrameResponse *)message;

	if (!msg->FrameStored && msg->StoredFrameCount != m_Camera.GetFrameCount())
	{
		CAM_LOG_ERROR("Server responded with unsuccessful frame stored!");
		return false;
	}

	CAM_LOG_INFO("Read back the frame response successfully!");
	return true;
}

void Client::ProcessFrame(Byte *frame, uint32 frame_size, uint32 frame_width, uint32 frame_height)
{
	// TODO
}

void Client::SendFrameToServer(Byte *frame, uint32 frame_size, uint32 frame_width, uint32 frame_height)
{
	ClientFrameMessage msg = {};
	msg.Header.Type = CLIENT_FRAME;
	msg.Header.Version = m_Version;
	msg.Frame = {};
	msg.Frame.FrameSize = frame_size;
	msg.Frame.FrameWidth = frame_width;
	msg.Frame.FrameHeight = frame_height;
	msg.Frame.Format = m_Camera.GetFormat();

	int32 bytesSent = m_Socket->Send(&msg, sizeof(msg), m_Host);
	CAM_LOG_INFO("Sending frame with {0} bytes. Actual message size: {1}", sizeof(msg), bytesSent);

	//std::this_thread::sleep_for(std::chrono::milliseconds(50));

	bytesSent = m_Socket->SendLarge(frame, frame_size, m_Host);
	CAM_LOG_INFO("Sending frame data with {0} bytes. Actual message size: {1}", frame_size, bytesSent);
}
