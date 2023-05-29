#include "Server.h"

#include <iostream>
#include <filesystem>

#include <miniz/miniz.h>

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

	//m_LocalVersion = Core::utils::GetLocalVersion(m_FileSystem, m_Config.TargetUpdatePath);
	m_LocalVersion = 101;
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

bool Server::LoadUpdateFile()
{
	// First check, if the update path is valid
	std::string update_path = m_Config.TargetBinaryPath;
	std::string update_file = update_path + "/update.zip";

	if (!m_FileSystem->DirectoryExists(update_path))
	{
		std::cerr << "Binary path from source does not exist! Please re-check your binary path or build the source first." << std::endl;
		return false;
	}

	// then delete an existing update file
	if (m_FileSystem->FileExists(update_file))
	{
		if (!m_FileSystem->RemoveFile(update_file))
		{
			std::cerr << "Could not delete the file " << update_file << std::endl;
			return false;
		}
	}

	// load the contents of the directory and store them into the zip file
	for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(update_path))
	{
		const std::filesystem::path p = entry.path();
		std::string zip_name = p.filename().string();
	
		// skip the archive itself
		if (p.string().find("update") != std::string::npos)
		{
			continue;
		}
	
		std::cout << "Adding " << zip_name.c_str() << "..." << std::endl;
	
		m_FileSystem->Open(p.string(), "r");
		int64 file_size = m_FileSystem->Size();
		Byte *data = new Byte[file_size];
		m_FileSystem->Read(data, (uint32)file_size);
		m_FileSystem->Close();
	
		mz_bool status = mz_zip_add_mem_to_archive_file_in_place(update_file.c_str(), zip_name.c_str(), data, file_size, nullptr, 0, MZ_BEST_COMPRESSION);
		if (!status)
		{
			std::cerr << "Could not put " << zip_name.c_str() << " into " << update_file.c_str() << std::endl;
			return false;
		}
	
		delete[] data;
		data = nullptr;
	}

	std::cout << "Added all files." << std::endl;

	// Load the whole ZIP file into memory
	m_FileSystem->Open(update_file, "r");
	int64 update_file_size = m_FileSystem->Size();
	m_UpdateFile.Alloc((uint32)update_file_size);
	m_FileSystem->Read(m_UpdateFile.Data, (uint32)update_file_size);
	m_FileSystem->Close();

	Core::Crypto::key_t private_key, public_key;
	if (m_FileSystem->FileExists(m_Config.PrivateKeyPath))
	{
		if (!m_FileSystem->ReadFile(m_Config.PublicKeyPath, public_key.Data, &public_key.Size))
		{
			std::cerr << "Could not read the public key!" << std::endl;
			return false;
		}
		
		if (!m_FileSystem->ReadFile(m_Config.PrivateKeyPath, private_key.Data, &private_key.Size))
		{
			std::cerr << "Could not read the private key!" << std::endl;
			return false;
		}
	}
	else
	{
		// First get the size needed for the keys and store them in the public_key and private_key variable
		m_Crypto->GenKeys(&public_key, &private_key);

		// then generate again
		if (!m_Crypto->GenKeys(&public_key, &private_key))
		{
			std::cerr << "Could not generate public/private key pair!" << std::endl;
			return false;
		}
	}

	// make the signature for the file
	if (!m_Crypto->SignSignature(m_UpdateSignature.Data, sizeof(m_UpdateSignature.Data), m_UpdateFile.Data, m_UpdateFile.Size, private_key.Data, private_key.Size))
	{
		std::cerr << "Could not sign the update!" << std::endl;
		return false;
	}

	if (m_FileSystem->FileExists(m_Config.SignaturePath))
	{
		if (!m_FileSystem->RemoveFile(m_Config.SignaturePath))
		{
			std::cerr << "Could not delete the old signature file!" << std::endl;
			return false;
		}
	}

	// Now write the security files
	if (!m_FileSystem->WriteFile(m_Config.PrivateKeyPath, private_key.Data, private_key.Size))
	{
		std::cerr << "Could not write private key file!" << std::endl;
		return false;
	}

	if (!m_FileSystem->WriteFile(m_Config.PublicKeyPath, public_key.Data, public_key.Size))
	{
		std::cerr << "Could not write public key file!" << std::endl;
		return false;
	}

	if (!m_FileSystem->WriteFile(m_Config.SignaturePath, m_UpdateSignature.Data, SIG_BYTES))
	{
		std::cerr << "Could not write the new signature!" << std::endl;
		return false;
	}

	return true;
}

void Server::StartFileWatcher()
{
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
	if (header->Type == MessageType::CLIENT_UPDATE_BEGIN)
	{
		if (len != sizeof(ClientUpdateBeginMessage))
		{
			return true;
		}

		ClientUpdateBeginMessage *msg = (ClientUpdateBeginMessage *)BUF;
		std::cout << "Received Update begin request with client token: " << msg->ClientToken << std::endl;

		int64 now_ms = Core::QueryMS();
		auto client = m_Clients->Insert(addr, now_ms);

		if (!client)
		{
			std::cerr << "Client could not be inserted!" << std::endl;
			return true;
		}

		if (!client->IsBandwidthAvailable(now_ms))
		{
			std::cerr << "Client has no bandwidth available!" << std::endl;
			return true;
		}

		if (msg->ServerToken != client->ServerToken)
		{
			std::cerr << "Server tokens did not match! Sending Server update token message..." << std::endl;
			ServerUpdateTokenMessage res = {};
			res.Header.Type = MessageType::SERVER_UPDATE_TOKEN;
			res.Header.Version = m_LocalVersion;
			res.ClientToken = msg->ClientToken;
			res.ServerToken = client->ServerToken;
			m_Socket->Send(&res, sizeof(res), addr);

			client->Bandwidth += sizeof(ServerUpdateTokenMessage);

			return true;
		}

		std::cout << "Sending update begin response..." << std::endl;
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

		ClientUpdatePieceMessage *msg = (ClientUpdatePieceMessage *)BUF;
		std::cout << "Received update piece request for update position: " << msg->PiecePos << std::endl;

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

		std::cout << "Sending update piece response..." << std::endl;
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
		std::cout << "Received server version request for version: " << msg->LocalVersion << std::endl;

		std::cout << "Sending server version response with version: " << m_LocalVersion << std::endl;
		ServerVersionInfoMessage res = {};
		res.Header.Type = MessageType::SERVER_RECEIVE_VERSION;
		res.Header.Version = m_LocalVersion;
		res.Version = m_LocalVersion;
		m_Socket->Send(&res, sizeof(res), addr);
	}

	return true;
}

