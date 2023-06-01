#include "Client.h"

#include <iostream>

Client::Client(const ClientConfig &config)
	: m_Config(config), m_Camera(false, 1280, 720)
{
	m_Socket = Core::Socket::Create();

	if (!m_Socket->Open())
	{
		std::cerr << "Socket could not be opened!" << std::endl;
	}

	if (!m_Socket->SetNonBlocking(true))
	{
		std::cerr << "Socket could not be set to non-blocking!" << std::endl;
	}

	m_Host = m_Socket->Lookup(m_Config.ServerIP, m_Config.Port);
	m_Version = Core::utils::GetLocalVersion("../../..");
	std::cout << "CameraClient version: " << m_Version << std::endl;
}

Client::~Client()
{
	Release();
}

void Client::Release()
{
	std::cout << "Releasing all resources." << std::endl;

	if (m_CameraThread.joinable())
		m_CameraThread.join();

	if (m_NetworkThread.joinable())
		m_NetworkThread.join();

	if (m_Socket)
	{
		std::cout << "Releasing socket" << std::endl;
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
	std::cout << "Sending connection start request to server..." << std::endl;
	ClientConnectionStartMessage msg = {};
	msg.Header.Type = CLIENT_CONNECTION_START;
	msg.Header.Version = m_Version;
	m_Socket->Send(&msg, sizeof(msg), m_Host);

	while (true)
	{
		static Byte BUF[65536];

		if (!m_Running && !m_SentConnectionCloseRequest)
		{
			// send request to server, that we want to close the connection
			std::cout << "Sending connection close request to server..." << std::endl;
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
			std::cerr << "Failed to receive data from network layer!" << std::endl;
			continue;
		}

		// ignore all messages from unknown senders
		if (addr.Value != m_Host.Value)
		{
			std::cerr << "Unknown sender!" << std::endl;
			continue;
		}

		if (len < sizeof(header_t))
		{
			std::cerr << "Unexpected header size!" << std::endl;
			continue;
		}

		header_t *header = (header_t *)BUF;

		if (header->Version != m_Version)
		{
			std::cerr << "Unexpected version encountered." << std::endl;
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
			std::cerr << "Specific message handler failed. Aborting." << std::endl;
			break;
		}

		// If we handled the server connecection close request successfully, quit from the network thread.
		if (header->Type == SERVER_CONNECTION_CLOSE && message_success)
		{
			m_NetworkThreadFinished = true;
			std::cout << "We got the connection close response from the server and it was successful. Closing Connection." << std::endl;
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

		m_Camera.ShowLive();
	}
}

bool Client::OnConnectionAccepted(Byte *message, uint32 length)
{
	if (length != sizeof(ServerConnectionStartResponse))
	{
		std::cerr << "Unexpected message size encountered." << std::endl;
		return false;
	}

	std::cout << "Trying to connect to server..." << std::endl;
	ServerConnectionStartResponse *msg = (ServerConnectionStartResponse *)message;



	std::cout << "Connected to server successfully!" << std::endl;
	return true;
}

bool Client::OnConnectionClosed(Byte *message, uint32 length)
{
	if (length != sizeof(ServerConnectionCloseResponse))
	{
		std::cerr << "Unexpected message size encountered." << std::endl;
		return false;
	}

	std::cout << "Trying to disconnect from server..." << std::endl;
	ServerConnectionCloseResponse *msg = (ServerConnectionCloseResponse *)message;



	std::cout << "Disconnected from server successfully!" << std::endl;
	return true;
}

bool Client::OnFrameResponse(Byte *message, uint32 length)
{
	if (length != sizeof(ServerFrameResponse))
	{
		std::cerr << "Unexpected message size encountered." << std::endl;
		return false;
	}

	std::cout << "Trying to read back the frame response from server..." << std::endl;
	ServerFrameResponse *msg = (ServerFrameResponse *)message;



	std::cout << "Read back the frame response successfully!" << std::endl;
	return true;
}
