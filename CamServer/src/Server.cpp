#include "Server.h"

#include <iostream>

#include "Messages.h"

Server::Server(const ServerConfig &config)
	: m_Config(config)
{
	m_Socket = Core::Socket::Create();
	m_Version = Core::utils::GetLocalVersion("../../..");
}

Server::~Server()
{
	delete m_Socket;
	m_Socket = nullptr;
}

void Server::Run()
{
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
	if (addrLen != sizeof(ClientConnectionStartMessage))
	{
		std::cerr << "Request size was not as expected!" << std::endl;
		return false;
	}

	std::cout << "Client " << clientAddr.Value  << " tries to connect!" << std::endl;

	ClientConnectionStartMessage *msg = (ClientConnectionStartMessage *)message;
	if (msg->Header.Version != m_Version)
	{
		std::cerr << "Version did not match with server version!" << std::endl;
		return false;
	}



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
	if (msg->Header.Version != m_Version)
	{
		std::cerr << "Version did not match with server version!" << std::endl;
		return false;
	}



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
	if (msg->Header.Version != m_Version)
	{
		std::cerr << "Version did not match with server version!" << std::endl;
		return false;
	}



	std::cout << "Server received frame from Client " << clientAddr.Value << " successfully!" << std::endl;
	return true;
}

