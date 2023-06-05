#include "Server.h"

#include <iostream>

#include "Messages.h"

Server::Server(const ServerConfig &config)
	: m_Config(config)
{
	m_Socket = Core::Socket::Create();
	m_Version = Core::utils::GetLocalVersion("../../..");
	std::cout << "CamServer version: " << m_Version << std::endl;
}

Server::~Server()
{
	if (m_FramePreviewThread.joinable())
	{
		m_FramePreviewThread.join();
	}

	for (uint32 i = 0; i < m_Frames.size(); ++i)
	{
		m_Frames[i].Frames.clear();
		m_Frames[i].Frames.shrink_to_fit();
	}

	m_Frames.clear();
	m_Frames.shrink_to_fit();

	delete m_Socket;
	m_Socket = nullptr;
}

void Server::Run()
{
	m_Running = true;

	for (;;)
	{
		m_Socket->Open();

		if (!m_Socket->Bind(m_Config.Port))
		{
			std::cerr << "Could not bind socket" << std::endl;
			break;
		}

		for (;;)
		{
			if (!Step())
			{
				break;
			}
		}

		std::cerr << "error: network interface failure." << std::endl;
		m_Socket->Close();

		Core::SleepMS(2000);
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
	if (header->Version != m_Version)
	{
		std::cerr << "Version did not match with server version!" << std::endl;
		return false;
	}

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
	if (addrLen != sizeof(ClientConnectionStartMessage))
	{
		std::cerr << "Request size was not as expected!" << std::endl;
		return false;
	}

	std::cout << "Client " << clientAddr.Value  << " tries to connect!" << std::endl;
	ClientConnectionStartMessage *msg = (ClientConnectionStartMessage *)message;

	bool client_connected = false;
	auto it = std::find(m_Frames.begin(), m_Frames.end(), clientAddr);
	if (it == m_Frames.end())
	{
		// No client registered yet
		ClientEntry client;
		client.Address = clientAddr;
		client.Frames = std::vector<cv::Mat>();
		client.FrameTitle = msg->FrameName;
		m_Frames.push_back(client);
		client_connected = true;
	}

	ServerConnectionStartResponse response = {};
	response.Header.Type = SERVER_CONNECTION_START;
	response.Header.Version = m_Version;
	response.ConnectionAccepted = client_connected;
	m_Socket->Send(&response, sizeof(response), clientAddr);

	std::cout << "Client " << clientAddr.Value << " connected successfully!" << std::endl;
	return true;
}

bool Server::OnClientDisconnected(Core::addr_t &clientAddr, Byte *message, int32 addrLen)
{
	if (addrLen != sizeof(ClientConnectionCloseMessage))
	{
		std::cerr << "Request size was not as expected!" << std::endl;
		return false;
	}

	std::cout << "Client " << clientAddr.Value << " tries to disconnect!" << std::endl;
	ClientConnectionCloseMessage *msg = (ClientConnectionCloseMessage *)message;

	bool client_removed = false;
	auto it = std::find(m_Frames.begin(), m_Frames.end(), clientAddr);
	if (it != m_Frames.end())
	{
		// Client was found, clear the entry
		it->Frames.clear();
		it->Frames.shrink_to_fit();

		m_Frames.erase(it);
		client_removed = true;
	}

	ServerConnectionCloseResponse response = {};
	response.Header.Type = SERVER_CONNECTION_CLOSE;
	response.Header.Version = m_Version;
	response.ConnectionClosed = client_removed;
	m_Socket->Send(&response, sizeof(response), clientAddr);

	std::cout << "Client " << clientAddr.Value << " disconnected successfully!" << std::endl;
	return true;
}

bool Server::OnClientFrame(Core::addr_t &clientAddr, Byte *message, int32 addrLen)
{
	if (addrLen != sizeof(ClientFrameMessage))
	{
		std::cerr << "Request size was not as expected!" << std::endl;
		return false;
	}

	std::cout << "Client " << clientAddr.Value << " tries to send a frame!" << std::endl;
	ClientFrameMessage *msg = (ClientFrameMessage *)message;

	bool frame_stored = false;
	uint32 frame_number = 0;
	auto it = std::find(m_Frames.begin(), m_Frames.end(), clientAddr);
	if (it != m_Frames.end())
	{
		// found the client entry, store the frame
		Byte *frame = msg->Frame.Frame;
		cv::Mat image(cv::Size(msg->Frame.FrameWidth, msg->Frame.FrameHeight), CV_8UC1, frame, cv::Mat::AUTO_STEP);

		it->Frames.push_back(image);
		frame_stored = true;
		frame_number = (uint32)it->Frames.size();
	}

	ServerFrameResponse response = {};
	response.Header.Type = SERVER_FRAME;
	response.Header.Version = m_Version;
	response.FrameStored = frame_stored;
	response.StoredFrameCount = frame_number;
	m_Socket->Send(&response, sizeof(response), clientAddr);

	std::cout << "Server received frame from Client " << clientAddr.Value << " successfully!" << std::endl;
	return true;
}

void Server::FramePreview()
{
	while (m_Running)
	{
		for (uint32 i = 0; i < m_Frames.size(); ++i)
		{
			ClientEntry &client = m_Frames[i];

			for (uint32 j = 0; j < client.Frames.size(); ++j)
			{
				cv::Mat &frame = client.Frames[j];
				std::string &name = client.FrameTitle;

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

