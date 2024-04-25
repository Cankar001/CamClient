#include "Server.h"

#include <iostream>

#include "Messages.h"

#include "Core/Log.h"

Server::Server(const ServerConfig &config)
	: m_Config(config)
{
	m_Socket = Core::Socket::Create();

	std::string cwd = "";
	Core::FileSystem::Get()->GetCurrentWorkingDirectory(&cwd);
	m_Version = Core::utils::GetLocalVersion(cwd);

	CAM_LOG_INFO("===================== CONFIG ===================================");
	CAM_LOG_INFO("IP                    : {}", config.ServerIP);
	CAM_LOG_INFO("Port                  : {}", config.Port);
	CAM_LOG_INFO("Video backup duration : {}", config.VideoBackupDuration);
	CAM_LOG_INFO("Current Server version: {}", m_Version);
	CAM_LOG_INFO("Current CWD           : {}", cwd);
	CAM_LOG_INFO("================================================================");
}

Server::~Server()
{
	if (m_FramePreviewThread.joinable())
	{
		m_FramePreviewThread.join();
	}

	for (uint32 i = 0; i < m_Clients.size(); ++i)
	{
		m_Clients[i].Frames.Clear();
	}

	m_Clients.clear();
	m_Clients.shrink_to_fit();

	delete m_Socket;
	m_Socket = nullptr;
}

void Server::Run()
{
	m_Running = true;
	CAM_LOG_INFO("Waiting for clients to connect...");

	for (;;)
	{
		if (!m_Socket->Open())
		{
			CAM_LOG_ERROR("Could not open socket!");
			break;
		}

		if (!m_Socket->Bind(m_Config.Port))
		{
			CAM_LOG_ERROR("Could not bind socket");
			break;
		}

		for (;;)
		{
			if (!Step())
			{
				break;
			}
		}

		CAM_LOG_ERROR("Network interface failure.");
		m_Socket->Close();

		Core::SleepMS(10);
	}

	m_Running = false;
}

void Server::StartFramePreviews()
{
	m_FramePreviewThread = std::thread(&Server::FramePreview, std::ref(*this));
}

bool Server::Step()
{
	static Byte BUF[65536];

	Core::addr_t addr;
	int32 len = m_Socket->Recv(BUF, sizeof(BUF), &addr);
	if (len < 0)
	{
		return false;
	}

	if (len < sizeof(header_t))
	{
		return true;
	}

	header_t *header = (header_t *)BUF;
	bool message_success = false;
	switch (header->Type)
	{
		case CLIENT_CONNECTION_START:
			message_success = OnClientConnected(addr, BUF, len);
			break;

		case CLIENT_CONNECTION_CLOSE:
			message_success = OnClientDisconnected(addr, BUF, len);
			break;

		case CLIENT_FRAME:
			message_success = OnClientFrame(addr, BUF, len);
			break;
	}

	return message_success;
}

bool Server::OnClientConnected(Core::addr_t &clientAddr, Byte *message, int32 addrLen)
{
	header_t *header = (header_t *)message;
	if (header->Version != m_Version)
	{
		CAM_LOG_ERROR("Version did not match with server version!");
		return false;
	}

	if (addrLen != sizeof(ClientConnectionStartMessage))
	{
		CAM_LOG_ERROR("Request size was not as expected!");
		return false;
	}

	CAM_LOG_DEBUG("Client {} tries to connect!", clientAddr.Value);
	ClientConnectionStartMessage *msg = (ClientConnectionStartMessage *)message;

	bool client_connected = false;
	auto it = std::find(m_Clients.begin(), m_Clients.end(), clientAddr);
	if (it == m_Clients.end())
	{
		// No client registered yet

		// Calculate the buffer size for X minutes
		uint32 fps = msg->FPS;
		uint32 minutes = m_Config.VideoBackupDuration;
		uint32 seconds = minutes * 60;
		uint32 frames = seconds * fps;
		CAM_LOG_DEBUG("Calculated frame count {0} for {1} minutes with {2} fps.", frames, minutes, fps);

		ClientEntry client(frames * sizeof(cv::Mat));
		client.Address = clientAddr;
		client.FrameTitle = msg->FrameName;
		m_Clients.push_back(client);
		client_connected = true;
	}

	ServerConnectionStartResponse response = {};
	response.Header.Type = SERVER_CONNECTION_START;
	response.Header.Version = m_Version;
	response.ConnectionAccepted = client_connected;
	m_Socket->Send(&response, sizeof(response), clientAddr);

	CAM_LOG_INFO("Client {} connected successfully!", clientAddr.Value);
	return true;
}

bool Server::OnClientDisconnected(Core::addr_t &clientAddr, Byte *message, int32 addrLen)
{
	header_t *header = (header_t *)message;
	if (header->Version != m_Version)
	{
		CAM_LOG_ERROR("Version did not match with server version!");
		return false;
	}

	if (addrLen != sizeof(ClientConnectionCloseMessage))
	{
		CAM_LOG_ERROR("Request size was not as expected!");
		return false;
	}

	CAM_LOG_DEBUG("Client {} tries to disconnect!", clientAddr.Value);
	ClientConnectionCloseMessage *msg = (ClientConnectionCloseMessage *)message;

	bool client_removed = false;
	auto it = std::find(m_Clients.begin(), m_Clients.end(), clientAddr);
	if (it != m_Clients.end())
	{
		// Client was found, clear the entry
		it->Frames.Clear();

		m_Clients.erase(it);
		client_removed = true;
	}

	ServerConnectionCloseResponse response = {};
	response.Header.Type = SERVER_CONNECTION_CLOSE;
	response.Header.Version = m_Version;
	response.ConnectionClosed = client_removed;
	m_Socket->Send(&response, sizeof(response), clientAddr);

	CAM_LOG_INFO("Client {} disconnected successfully!", clientAddr.Value);
	return true;
}

bool Server::OnClientFrame(Core::addr_t &clientAddr, Byte *message, int32 addrLen)
{
	header_t *header = (header_t *)message;
	if (header->Version != m_Version)
	{
		CAM_LOG_ERROR("Version did not match with server version!");
		return false;
	}

	if (addrLen != sizeof(ClientFrameMessage))
	{
		CAM_LOG_ERROR("Request size was not as expected!");
		return false;
	}

	CAM_LOG_DEBUG("Client {} tries to send a frame!", clientAddr.Value);
	ClientFrameMessage *msg = (ClientFrameMessage *)message;

	bool frame_stored = false;
	uint32 frame_number = 0;
	auto it = std::find(m_Clients.begin(), m_Clients.end(), clientAddr);
	if (it != m_Clients.end())
	{
		Byte *frame = new Byte[msg->Frame.FrameSize];
		uint32 frame_size = msg->Frame.FrameSize;

		Core::addr_t current_client;
		int32 bytes_received = m_Socket->RecvLarge(frame, frame_size, &current_client);
	//	assert(frame_size == bytes_received);
		
		it->FrameWidth = msg->Frame.FrameWidth;
		it->FrameHeight = msg->Frame.FrameHeight;

		cv::Mat image(cv::Size(msg->Frame.FrameWidth, msg->Frame.FrameHeight), msg->Frame.Format, frame, cv::Mat::AUTO_STEP);

	//	cv::Mat image(msg->Frame.FrameHeight, msg->Frame.FrameWidth, msg->Frame.Format);
	//	cv::Mat image(msg->Frame.FrameHeight, msg->Frame.FrameWidth, CV_8UC3);

		int32 baseIndex = 0;
		for (int32 i = 0; i < image.rows; i++)
		{
			for (int32 j = 0; j < image.cols; j++)
			{
				image.at<cv::Vec3b>(i, j) = cv::Vec3b(frame[baseIndex + 0], frame[baseIndex + 1], frame[baseIndex + 2]);
				baseIndex += 3;
			}
		}
		
		it->Frames.Push(image);
		frame_stored = true;
		frame_number = it->Frames.Size();
	}

	ServerFrameResponse response = {};
	response.Header.Type = SERVER_FRAME;
	response.Header.Version = m_Version;
	response.FrameStored = frame_stored;
	response.StoredFrameCount = frame_number;
	m_Socket->Send(&response, sizeof(response), clientAddr);

	CAM_LOG_INFO("Server received frame from Client {} successfully!", clientAddr.Value);
	return true;
}

void Server::FramePreview()
{
	while (m_Running)
	{
		for (uint32 i = 0; i < m_Clients.size(); ++i)
		{
			ClientEntry &client = m_Clients[i];

			for (uint32 j = 0; j < client.Frames.Size(); ++j)
			{
				cv::Mat frame = client.Frames.Front();
				std::string &name = client.FrameTitle;
				uint32 frame_width = client.FrameWidth;
				uint32 frmae_height = client.FrameHeight;

				cv::namedWindow(name.c_str(), cv::WND_PROP_FULLSCREEN);
				cv::setWindowProperty(name.c_str(), cv::WND_PROP_FULLSCREEN, cv::WND_PROP_FULLSCREEN);
				cv::imshow(name.c_str(), frame);

				char key = cv::waitKey(1);
				if (key == 'q')
				{
					cv::destroyWindow(name.c_str());
				}
			}
		}
	}
}

