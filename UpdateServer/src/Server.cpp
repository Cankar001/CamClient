#include "Server.h"

#include <iostream>

#include "Utils/Utils.h"

Server::Server(const ServerConfig &config)
	: m_Config(config)
{
	m_LastUpdateCheckMS = 0;
	m_LastUpdateWriteMS = 0;
	m_UpdateSignature = {};

	m_Socket = Core::Socket::Create();
	m_Crypto = Core::Crypto::Create();
	m_FileSystem = Core::FileSystem::Create();
	m_IPTable = new Core::IPTable();
	m_Clients = new Core::Clients(m_Crypto, m_IPTable);

	m_LocalVersion = Core::utils::GetLocalVersion(m_FileSystem, m_Config.TargetUpdatePath);
	std::cout << "Current Server version: " << m_LocalVersion << std::endl;
}

Server::~Server()
{
	delete m_Clients;
	m_Clients = nullptr;

	delete m_IPTable;
	m_IPTable = nullptr;

	delete m_FileSystem;
	m_FileSystem = nullptr;

	delete m_Crypto;
	m_Crypto = nullptr;

	delete m_Socket;
	m_Socket = nullptr;
}

void Server::Run()
{
	for (;;)
	{
		m_Socket->Open();

		if (!m_Socket->Bind(m_Config.ServerPort))
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

		Core::SleepMS(5000);
	}

	std::cerr << "error: socket creation failed." << std::endl;
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

	auto header = (header_t *)BUF;
	if (header->Version != m_LocalVersion)
	{
		std::cerr << "Header version does not match the server version!" << std::endl;
		return true;
	}

	if (header->Type == MessageType::CLIENT_UPDATE_BEGIN)
	{
		if (len != sizeof(ClientUpdateBeginMessage))
		{
			return true;
		}

		ClientUpdateBeginMessage *msg = (ClientUpdateBeginMessage *)BUF;

		int64 now_ms = Core::QueryMS();
		auto client = m_Clients->Insert(addr, now_ms);

		if (!client)
		{
			return true;
		}

		if (!client->IsBandwidthAvailable(now_ms))
		{
			return true;
		}

		if (msg->ServerToken != client->ServerToken)
		{
			ServerUpdateTokenMessage res = {};
			res.Header.Type = MessageType::SERVER_UPDATE_TOKEN;
			res.Header.Version = m_LocalVersion;
			res.ClientToken = msg->ClientToken;
			res.ServerToken = client->ServerToken;
			m_Socket->Send(&res, sizeof(res), addr);

			client->Bandwidth += sizeof(ServerUpdateTokenMessage);

			return true;
		}

		if (!UpdateRefresh(now_ms))
		{
			return true;
		}

		if (msg->ClientVersion == m_LocalVersion)
		{
			return true;
		}

		ServerUpdateBeginMessage res;
		memset(&res, 0, sizeof(res));

		res.Header.Version = m_LocalVersion;
		res.Header.Type = MessageType::SERVER_UPDATE_BEGIN;
		res.ClientToken = msg->ClientToken;
		res.ServerToken = client->ServerToken;
		res.ServerVersion = m_LocalVersion;
		res.UpdateSize = m_UpdateFile.Size;
		res.UpdateSignature = m_UpdateSignature;
		m_Socket->Send(&res, sizeof(res), addr);

		client->Bandwidth += sizeof(ServerUpdateBeginMessage);
	}
	else if (header->Type == MessageType::CLIENT_UPDATE_PIECE)
	{
		if (len != sizeof(ClientUpdatePieceMessage))
		{
			return true;
		}

		int64 now_ms = Core::QueryMS();

		if (!UpdateRefresh(now_ms))
		{
			return true;
		}

		ClientUpdatePieceMessage *msg = (ClientUpdatePieceMessage *)BUF;

		if (msg->ServerVersion != m_LocalVersion)
		{
			return true;
		}

		if (msg->PiecePos >= m_UpdateFile.Size)
		{
			return true;
		}

		auto client = m_Clients->Insert(addr, now_ms);

		if (!client)
		{
			return true;
		}

		if (!client->IsBandwidthAvailable(now_ms))
		{
			return true;
		}

		if (msg->ServerToken != client->ServerToken)
		{
			return true;
		}

		ServerUpdatePieceMessage res = {};
		res.Header.Version = m_LocalVersion;
		res.Header.Type = MessageType::SERVER_UPDATE_PIECE;
		res.ClientToken = msg->ClientToken;
		res.ServerToken = client->ServerToken;
		res.ServerVersion = m_LocalVersion;
		res.PiecePos = msg->PiecePos;
		res.PieceSize = (uint16)Core::utils::Min<uint32>(m_UpdateFile.Size - msg->PiecePos, PIECE_BYTES);

		char send_buf[sizeof(res) + PIECE_BYTES];
		memcpy(send_buf, &res, sizeof(res));
		memcpy(send_buf + sizeof(res), m_UpdateFile.Data + msg->PiecePos, res.PieceSize);

		uint32 send_bytes = sizeof(res) + res.PieceSize;
		m_Socket->Send(send_buf, send_bytes, addr);

		client->Bandwidth += send_bytes;
	}
	else if (header->Type == MessageType::CLIENT_REQUEST_VERSION)
	{
		if (len != sizeof(ClientWantsVersionMessage))
		{
			return true;
		}

		ClientWantsVersionMessage *msg = (ClientWantsVersionMessage *)BUF;
		uint32 client_version = msg->LocalVersion;
		
		ServerVersionInfoMessage res = {};
		res.Header.Type = MessageType::SERVER_RECEIVE_VERSION;
		res.Header.Version = m_LocalVersion;
		res.Version = m_LocalVersion;
		m_Socket->Send(&res, sizeof(res), addr);
	}

	return true;
}

bool Server::UpdateRefresh(int64 now_ms)
{
	if (now_ms - m_LastUpdateCheckMS > 15000)
	{
		m_LastUpdateCheckMS = now_ms;

		/*
		int64 file_time_ms = Core::FileSystem::GetWriteMS(update_sig_path);
		if (file_time_ms != m_LastUpdateWriteMS)
		{
			m_LastUpdateWriteMS = file_time_ms;

			m_UpdateFile.Free();
			if (filesystem::read(&m_UpdateFile, m_Config.TargetUpdatePath + "/update.zip"))
			{
				m_UpdateVersion = Core::Crc32(m_UpdateFile.Data, m_UpdateFile.Size);
			}

			if (filesystem::file_t file; file.open(GENERIC_READ, OPEN_EXISTING, update_sig_path))
			{
				file.read(&update_sig, sizeof(update_sig));
			}
		}
	*/
	}

	return (m_UpdateFile.Data != nullptr);
}

