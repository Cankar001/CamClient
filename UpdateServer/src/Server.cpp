#include "Server.h"

#include <iostream>
#include <filesystem>

#include "Utils/Utils.h"
#include "Utils/ZipArchive.h"
#include "Core/Log.h"

Server::Server(const ServerConfig &config)
	: m_Config(config)
{
	m_LastUpdateCheckMS = 0;
	m_LastUpdateWriteMS = 0;
	m_UpdateSignature = {};

	m_Socket = Core::Socket::Create();
	m_Crypto = Core::Crypto::Create();
	m_IPTable = new Core::IPTable();
	m_Clients = new Core::Clients(m_Crypto, m_IPTable);

	//m_LocalVersion = Core::utils::GetLocalVersion(m_Config.TargetSourcePath);
	m_LocalVersion = 101;

	std::string cwd = "";
	Core::FileSystem::Get()->GetCurrentWorkingDirectory(&cwd);

	CAM_LOG_INFO("===================== CONFIG ===================================");
	CAM_LOG_INFO("Private key path      : {}", config.PrivateKeyPath);
	CAM_LOG_INFO("Public key path       : {}", config.PublicKeyPath);
	CAM_LOG_INFO("IP                    : {}", config.ServerIP);
	CAM_LOG_INFO("Port                  : {}", config.ServerPort);
	CAM_LOG_INFO("Signature path        : {}", config.SignaturePath);
	CAM_LOG_INFO("Target binary path    : {}", config.TargetBinaryPath);
	CAM_LOG_INFO("Target source path    : {}", config.TargetSourcePath);
	CAM_LOG_INFO("Current Server version: {}", m_LocalVersion);
	CAM_LOG_INFO("Current CWD           : {}", cwd);
	CAM_LOG_INFO("================================================================");
}

Server::~Server()
{
	Core::FileSystemWatcher::Stop();

	delete m_Clients;
	m_Clients = nullptr;

	delete m_IPTable;
	m_IPTable = nullptr;

	delete m_Crypto;
	m_Crypto = nullptr;

	delete m_Socket;
	m_Socket = nullptr;
}

void Server::Run()
{
	CAM_LOG_INFO("Waiting for clients to connect...");

	for (;;)
	{
		m_Socket->Open();

		if (!m_Socket->Bind(m_Config.ServerPort))
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

		CAM_LOG_ERROR("network interface failure.");
		m_Socket->Close();

		Core::SleepMS(10);
	}

	CAM_LOG_ERROR("socket creation failed.");
}

bool Server::LoadUpdateFile(bool forceDeleteSignature, bool skipDebugFiles)
{
	// First check, if the update path is valid
	std::string update_path = m_Config.TargetBinaryPath;
	std::string update_file = update_path + "/update.zip";

	if (!Core::FileSystem::Get()->DirectoryExists(update_path))
	{
		CAM_LOG_ERROR("Binary path from source does not exist! Please re-check your binary path or build the source first.");
		return false;
	}

	CAM_LOG_INFO("Generating new update package at location {} ...", update_file);

	// then delete an existing update file
	if (Core::FileSystem::Get()->FileExists(update_file))
	{
		if (!Core::FileSystem::Get()->RemoveFile(update_file))
		{
			CAM_LOG_ERROR("Could not delete the file {}", update_file);
			return false;
		}
		else
		{
			CAM_LOG_INFO("Deleted existing packaged update from disk.");
		}
	}

	// load the contents of the directory and store them into the zip file
	std::vector<Core::ZipFile> files;
	for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(update_path))
	{
		const std::filesystem::path p = entry.path();
		std::string zip_name = p.filename().string();
		std::string current_file_name = p.string();
	
		// skip the archive itself
		if (current_file_name.find("update") != std::string::npos)
		{
			CAM_LOG_INFO("Skipping update file.");
			continue;
		}

		if (current_file_name.find("logs") != std::string::npos)
		{
			CAM_LOG_INFO("Skipping logs.");
			continue;
		}

		if (skipDebugFiles)
		{
			if (current_file_name.find(".pdb") != std::string::npos)
			{
				CAM_LOG_INFO("Skipping debug file {}", zip_name);
				continue;
			}
		}
	
		CAM_LOG_INFO("Adding {}...", zip_name);

		uint32 file_size = 0;
		Byte *data = Core::FileSystem::Get()->ReadFile(current_file_name, &file_size);
		if (!data)
		{
			CAM_LOG_ERROR("Could not read file {}!", current_file_name);
			return false;
		}
		
		Core::ZipFile file;
		file.Name = std::filesystem::path(current_file_name).filename().string();
		file.Path = current_file_name;
		file.Buffer = data;
		file.BufferSize = file_size;
		files.push_back(file);
	}

	CAM_LOG_INFO("Added all files.");

	// Now trying to store the whole contents into the zip file
	CAM_LOG_INFO("Writing zip file to memory...");
	Core::ZipArchive archive;
	if (!archive.Store(files, update_file))
	{
		CAM_LOG_ERROR("Could not save the zip file!");
		return false;
	}

	CAM_LOG_INFO("Zip file written successfully to {}", update_file);

	// Cleanup the memory.
	for (auto &file : files)
	{
		delete[] file.Buffer;
		file.Buffer = nullptr;
	}

	// Load the whole ZIP file into memory
	m_UpdateFile.Data = Core::FileSystem::Get()->ReadFile(update_file, &m_UpdateFile.Size);
	
	if (!m_UpdateFile.Data)
	{
		CAM_LOG_ERROR("Could not read back in the update file!");
		return false;
	}

	if (forceDeleteSignature)
	{
		if (Core::FileSystem::Get()->FileExists(m_Config.PrivateKeyPath))
		{
			if (!Core::FileSystem::Get()->RemoveFile(m_Config.PrivateKeyPath))
			{
				CAM_LOG_ERROR("Could not remove private key!");
				return false;
			}
		}

		if (Core::FileSystem::Get()->FileExists(m_Config.PublicKeyPath))
		{
			if (!Core::FileSystem::Get()->RemoveFile(m_Config.PublicKeyPath))
			{
				CAM_LOG_ERROR("Could not remove public key!");
				return false;
			}
		}

		if (Core::FileSystem::Get()->FileExists(m_Config.SignaturePath))
		{
			if (!Core::FileSystem::Get()->RemoveFile(m_Config.SignaturePath))
			{
				CAM_LOG_ERROR("Could not remove signature!");
				return false;
			}
		}
	}

	Core::Crypto::key_t private_key, public_key;
	if (Core::FileSystem::Get()->FileExists(m_Config.PrivateKeyPath))
	{
		Byte *public_key_data = Core::FileSystem::Get()->ReadFile(m_Config.PublicKeyPath, &public_key.Size);
		memcpy(public_key.Data, public_key_data, public_key.Size);
		delete[] public_key_data;
		public_key_data = nullptr;

		if (!public_key.Data)
		{
			CAM_LOG_ERROR("Could not read the public key!");
			return false;
		}
		
		Byte *private_key_data = Core::FileSystem::Get()->ReadFile(m_Config.PrivateKeyPath, &private_key.Size);
		memcpy(private_key.Data, private_key_data, private_key.Size);
		delete[] private_key_data;
		private_key_data = nullptr;

		if (!private_key.Data)
		{
			CAM_LOG_ERROR("Could not read the private key!");
			return false;
		}
	}
	else
	{
		if (!m_Crypto->GenKeys(&public_key, &private_key))
		{
			CAM_LOG_ERROR("Could not generate public/private key pair!");
			return false;
		}
	}

	// make the signature for the file
	if (!m_Crypto->SignSignature(
		m_UpdateSignature.Data, 
		sizeof(m_UpdateSignature.Data), 
		m_UpdateFile.Data, 
		m_UpdateFile.Size, 
		private_key.Data, 
		private_key.Size))
	{
		CAM_LOG_ERROR("Could not sign the update!");
		return false;
	}

	if (Core::FileSystem::Get()->FileExists(m_Config.SignaturePath))
	{
		if (!Core::FileSystem::Get()->RemoveFile(m_Config.SignaturePath))
		{
			CAM_LOG_ERROR("Could not delete the old signature file!");
			return false;
		}
	}

	// Now write the security files
	if (!Core::FileSystem::Get()->FileExists(m_Config.PrivateKeyPath))
	{
		if (!Core::FileSystem::Get()->WriteFile(m_Config.PrivateKeyPath, private_key.Data, private_key.Size))
		{
			CAM_LOG_ERROR("Could not write private key file!");
			return false;
		}
	}

	if (!Core::FileSystem::Get()->WriteFile(m_Config.PublicKeyPath, public_key.Data, public_key.Size))
	{
		CAM_LOG_ERROR("Could not write public key file!");
		return false;
	}

	if (!Core::FileSystem::Get()->WriteFile(m_Config.SignaturePath, m_UpdateSignature.Data, SIG_BYTES))
	{
		CAM_LOG_ERROR("Could not write the new signature!");
		return false;
	}

	m_PublicKey.Size = public_key.Size;
	memcpy(m_PublicKey.Data, public_key.Data, sizeof(public_key.Data));
	CAM_LOG_INFO("Loaded update with size {}", m_UpdateFile.Size);
	
	return true;
}

void Server::StartFileWatcher()
{
	Server *instance = this;
	Core::FileSystemWatcher::Start(m_Config.TargetBinaryPath, [instance](const Core::FileSystemWatcherContext &context) mutable
	{
		CAM_LOG_INFO("Something happened with file {}", context.FilePath);
		switch (context.Action)
		{
			case Core::FileSystemWatcherAction::Added:
			case Core::FileSystemWatcherAction::Modified:
			case Core::FileSystemWatcherAction::Removed:
			case Core::FileSystemWatcherAction::Renamed:
				instance->m_UpdateFile.Free();
				instance->m_UpdateSignature = {};
				instance->LoadUpdateFile(true);
				break;
		}
	});
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
		CAM_LOG_DEBUG("Received Update begin request with client token: {}", msg->ClientToken);

		int64 now_ms = Core::QueryMS();
		Core::Clients::Node *client = m_Clients->Insert(addr, now_ms);

		if (!client)
		{
			CAM_LOG_ERROR("Client could not be inserted!");
			return true;
		}

		if (!client->IsBandwidthAvailable(now_ms))
		{
		//	CAM_LOG_ERROR("Client has no bandwidth available!");
			return true;
		}

		if (msg->ServerToken != client->ServerToken)
		{
			CAM_LOG_WARN("Server tokens did not match! Sending Server update token message...");
			ServerUpdateTokenMessage res = {};
			res.Header.Type = MessageType::SERVER_UPDATE_TOKEN;
			res.Header.Version = m_LocalVersion;
			res.ClientToken = msg->ClientToken;
			res.ServerToken = client->ServerToken;
			m_Socket->Send(&res, sizeof(res), addr);

			client->Bandwidth += sizeof(ServerUpdateTokenMessage);

			return true;
		}

		CAM_LOG_DEBUG("Sending update begin response...");
		ServerUpdateBeginMessage res;
		memset(&res, 0, sizeof(res));

		res.Header.Version = m_LocalVersion;
		res.Header.Type = MessageType::SERVER_UPDATE_BEGIN;
		res.ClientToken = msg->ClientToken;
		res.ServerToken = client->ServerToken;
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

		if (msg->PiecePos >= m_UpdateFile.Size)
		{
			CAM_LOG_ERROR("The request position was larger than the file!");
			return true;
		}

		Core::Clients::Node *client = m_Clients->Insert(addr, now_ms);

		if (!client)
		{
			CAM_LOG_ERROR("Could not find the client for addr {0}, port {1}", addr.Host, addr.Port);
			return true;
		}

		if (!client->IsBandwidthAvailable(now_ms))
		{
		//	CAM_LOG_ERROR("Client has no bandwidth available!");
			return true;
		}

		if (msg->ServerToken != client->ServerToken)
		{
			CAM_LOG_ERROR("Server tokens did not match!");
			return true;
		}

		CAM_LOG_DEBUG("Sending update: {0} / {1}", msg->PiecePos, m_UpdateFile.Size);
		ServerUpdatePieceMessage res = {};
		res.Header.Version = m_LocalVersion;
		res.Header.Type = MessageType::SERVER_UPDATE_PIECE;
		res.ClientToken = msg->ClientToken;
		res.ServerToken = client->ServerToken;
		res.PiecePos = msg->PiecePos;
		res.PieceSize = (uint16)Core::utils::Min<uint32>(m_UpdateFile.Size - msg->PiecePos, PIECE_BYTES);

		char* send_buf = (char*)malloc(sizeof(res) + res.PieceSize);
		if (!send_buf)
		{
			CAM_LOG_ERROR("Failed to allocate enough space for the update package!");
			return true;
		}

		memcpy(send_buf, &res, sizeof(res));
		memcpy(send_buf + sizeof(res), m_UpdateFile.Data + msg->PiecePos, res.PieceSize);

		uint32 send_bytes = sizeof(res) + res.PieceSize;
		m_Socket->Send(send_buf, send_bytes, addr);

		free(send_buf);
		send_buf = nullptr;
		client->Bandwidth += send_bytes;
	}
	else if (header->Type == MessageType::CLIENT_REQUEST_VERSION)
	{
		if (len != sizeof(ClientWantsVersionMessage))
		{
			CAM_LOG_ERROR("ClientWantsVersionMessage: Unexpected message size.");
			return true;
		}

		ClientWantsVersionMessage *msg = (ClientWantsVersionMessage *)BUF;
		uint32 client_version = msg->LocalVersion;

		CAM_LOG_DEBUG("Received server version request for version: {}", msg->LocalVersion);
		CAM_LOG_DEBUG("Sending server version response with version: {}", m_LocalVersion);

		ServerVersionInfoMessage res = {};
		res.Header.Type = MessageType::SERVER_RECEIVE_VERSION;
		res.Header.Version = m_LocalVersion;
		res.Version = m_LocalVersion;
		res.PublicKey.Size = m_PublicKey.Size;
		memcpy(res.PublicKey.Data, m_PublicKey.Data, m_PublicKey.Size);
		m_Socket->Send(&res, sizeof(res), addr);
	}

	return true;
}

