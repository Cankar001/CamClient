#include "Client.h"

#include <iostream>

#include "Messages.h"

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
}

Client::~Client()
{
	delete m_Socket;
	m_Socket = nullptr;
}

void Client::Run()
{
	m_NetworkThread = std::thread(&Client::NetworkLoop, std::ref(*this));
	m_CameraThread = std::thread(&Client::CameraLoop, std::ref(*this));
}

void Client::NetworkLoop()
{
	while (m_Running)
	{
		static Byte BUF[65536];

		Core::addr_t addr;
		int32 len = m_Socket->Recv(BUF, sizeof(BUF), &addr);
		if (len < 0)
		{
			return;
		}

		// ignore all messages from unknown senders
		if (addr.Value != m_Host.Value)
		{
			continue;
		}

		if (len < sizeof(header_t))
		{
			continue;
		}

		header_t *header = (header_t *)BUF;
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
	}
}

void Client::CameraLoop()
{
	while (m_Running)
	{

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
	if (msg->Header.Version != m_Version)
	{
		std::cerr << "Unexpected version encountered." << std::endl;
		return false;
	}



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
	if (msg->Header.Version != m_Version)
	{
		std::cerr << "Unexpected version encountered." << std::endl;
		return false;
	}



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
	if (msg->Header.Version != m_Version)
	{
		std::cerr << "Unexpected version encountered." << std::endl;
		return false;
	}



	std::cout << "Read back the frame response successfully!" << std::endl;
	return true;
}
