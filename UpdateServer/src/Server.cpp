#include "Server.h"

#include <iostream>

Server::Server(const ServerConfig &config)
	: m_Config(config)
{
	m_LastUpdateCheckMS = 0;
	m_LastUpdateWriteMS = 0;
	m_UpdateVersion = 0;
	m_UpdateSignature = {};

	m_Socket = Core::Socket::Create();
	m_Crypto = Core::Crypto::Create();
	m_FileSystem = Core::FileSystem::Create();
	m_Clients = Core::Clients(m_Crypto, &m_IPTable);
}

Server::~Server()
{
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
	if (header->Version != m_UpdateVersion)
	{
		std::cerr << "Header version does not match the server version!" << std::endl;
		return true;
	}

	if (header->Type == MessageType::CLIENT_UPDATE_BEGIN)
	{
		if (len != sizeof(ClientUpdateBeginMessage))
		{
			return;
		}

		ClientUpdateBeginMessage *msg = (ClientUpdateBeginMessage *)BUF;

		// You probably want to check that client_version isn't 0 here, but we use that for demoing.

		int64 now_ms = Core::QueryMS();
		auto client = m_Clients.Insert(addr, now_ms);

		if (!client)
		{
			return;
		}

		if (!client->IsBandwidthAvailable(now_ms))
		{
			return;
		}

		if (msg->ServerToken != client->ServerToken)
		{
			ServerUpdateTokenMessage res = {};
			res.Header.Type = MessageType::SERVER_UPDATE_TOKEN;
			res.Header.Version = VERSION;
			res.ClientToken = msg->ClientToken;
			res.ServerToken = client->ServerToken;
			m_Socket->Send(&res, sizeof(res), addr);

			client->Bandwidth += sizeof(ServerUpdateTokenMessage);

			return;
		}

		if (!UpdateRefresh(now_ms))
		{
			return;
		}

		if (msg->ClientVersion == m_UpdateVersion)
		{
			return;
		}

		ServerUpdateBeginMessage res;
		memset(&res, 0, sizeof(res));

		res.Header.Version = VERSION;
		res.Header.Type = MessageType::SERVER_UPDATE_BEGIN;
		res.ClientToken = msg->ClientToken;
		res.ServerToken = client->ServerToken;
		res.ServerVersion = m_UpdateVersion;
		res.UpdateSize = m_UpdateFile.Size;
		res.UpdateSignature = m_UpdateSignature;
		m_Socket->Send(&res, sizeof(res), addr);

		client->Bandwidth += sizeof(ServerUpdateBeginMessage);
	}
	else if (header->Type == MessageType::CLIENT_UPDATE_PIECE)
	{
		if (len != sizeof(ClientUpdatePieceMessage))
		{
			return;
		}

		int64 now_ms = Core::QueryMS();

		if (!UpdateRefresh(now_ms))
		{
			return;
		}

		ClientUpdatePieceMessage *msg = (ClientUpdatePieceMessage *)BUF;

		if (msg->ServerVersion != m_UpdateVersion)
		{
			return;
		}

		if (msg->PiecePos >= m_UpdateFile.Size)
		{
			return;
		}

		auto client = m_Clients.Insert(addr, now_ms);

		if (!client)
		{
			return;
		}

		if (!client->IsBandwidthAvailable(now_ms))
		{
			return;
		}

		if (msg->ServerToken != client->ServerToken)
		{
			return;
		}

		ServerUpdatePieceMessage res = {};
		res.Header.Version = VERSION;
		res.Header.Type = MessageType::SERVER_UPDATE_PIECE;
		res.ClientToken = msg->ClientToken;
		res.ServerToken = client->ServerToken;
		res.ServerVersion = m_UpdateVersion;
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
			return;
		}

	//	ClientWantsVersionMessage *msg = (ClientWantsVersionMessage *)BUF;
		
		ServerVersionInfoMessage res = {};
		res.Header.Type = MessageType::SERVER_RECEIVE_VERSION;
		res.Header.Version = VERSION;
		res.Version = m_UpdateVersion;
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
	}
	*/

	return (m_UpdateFile.Data != nullptr);
}

