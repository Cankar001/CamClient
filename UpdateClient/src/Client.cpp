#include "Client.h"

#include <iostream>

namespace utils
{
	static bool HasMacroInText(const std::string &str, const std::string &macro)
	{
		size_t pos = str.find("#define " + macro);
		if (pos == std::string::npos)
		{
			return false;
		}

		return true;
	}

	static std::string GetMacroFromText(const std::string &str, const std::string &macro)
	{
		std::string macro_search_text = "#define " + macro;
		size_t pos = str.find(macro_search_text);
		if (pos == std::string::npos)
		{
			return "";
		}

		std::string result = str;
		result = result.substr(pos + macro_search_text.size() + 1);

		pos = result.find("\n");
		if (pos == std::string::npos)
		{
			return result;
		}

		result = result.substr(0, pos);
		return result;
	}

	static uint32 GetLocalVersion(Core::FileSystem *filesystem, const std::string &rootPath)
	{
		std::string local_path = rootPath + "/src/CamVersion.h";
		if (!filesystem->Open(local_path, "r"))
		{
			std::cerr << "Could not open file " << local_path.c_str() << std::endl;
			return 0;
		}

		std::string version_content = "";
		if (!filesystem->ReadTextFile(&version_content))
		{
			std::cerr << "Could not read version file!" << std::endl;
			return 0;
		}

		std::string local_version_str = utils::GetMacroFromText(version_content, "CAM_VERSION");
		filesystem->Close();
		return std::stoi(local_version_str);
	}
}

Client::Client(const ClientConfig &config)
	: m_Config(config)
{
	m_Socket = Core::Socket::Create();
	m_FileSystem = Core::FileSystem::Create();
	m_Crypto = Core::Crypto::Create();

	if (!m_Socket->Open())
	{
		std::cerr << "Socket could not be opened!" << std::endl;
	}

	if (!m_Socket->SetNonBlocking(true))
	{
		std::cerr << "Socket could not be set to non-blocking!" << std::endl;
	}

	m_Host = m_Socket->Lookup(m_Config.ServerIP, m_Config.Port);
	Reset();
}

Client::~Client()
{
	delete m_Crypto;
	m_Crypto = nullptr;

	delete m_FileSystem;
	m_FileSystem = nullptr;

	delete m_Socket;
	m_Socket = nullptr;
}

void Client::RequestServerVersion()
{
	// First load the local client version
	uint32 local_version = utils::GetLocalVersion(m_FileSystem, m_Config.UpdateTargetPath);
	if (!local_version)
	{
		std::cerr << "Could not retrieve the local version!" << std::endl;
		return;
	}

	std::cout << "Local version: " << local_version << std::endl;
	m_LocalVersion = local_version;

	// Then request the latest client version from the server
	ClientWantsVersionMessage message = {};
	message.LocalVersion = local_version;
	message.ClientVersion = 0;
	message.Header.Type = MessageType::CLIENT_REQUEST_VERSION;
	message.Header.Version = local_version;
	m_Socket->Send(&message, sizeof(message), m_Host);
	m_Status.Code = ClientStatusCode::NONE;
}

void Client::Run()
{
	// When running for the first time, request the server version
	RequestServerVersion();

	for (;;)
	{
		if (m_Status.Code == ClientStatusCode::UP_TO_DATE)
		{
			// TODO: start CamClient application

			m_Status.Code = ClientStatusCode::NONE;
		}

		MessageLoop();
	}
}

void Client::MessageLoop()
{
	for (;;)
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

		header_t *header = (header_t*)BUF;
		if (header->Type == MessageType::SERVER_RECEIVE_VERSION)
		{
			if (len != sizeof(ServerVersionInfoMessage))
			{
				return;
			}

			ServerVersionInfoMessage *msg = (ServerVersionInfoMessage *)BUF;
			if (msg->Version != m_LocalVersion)
			{
				// The versions are different, we need an update
				m_Status.Code = ClientStatusCode::NEEDS_UPDATE;
				m_ClientVersion = msg->Version;
			}
			else
			{
				// The versions are the same, we have the latest version
				m_Status.Code = ClientStatusCode::UP_TO_DATE;
			}
		}
		else if (header->Type == MessageType::SERVER_UPDATE_BEGIN)
		{
			if (len != sizeof(ServerUpdateBeginMessage))
			{
				return;
			}

			ServerUpdateBeginMessage *msg = (ServerUpdateBeginMessage *)BUF;
			
			// Verify that the update size is reasonable (<100MB).
			if (msg->UpdateSize == 0 || msg->UpdateSize >= 100000000)
			{
				return;
			}

			// Allocate space for update data.
			if (!m_UpdateData.Alloc(msg->UpdateSize))
			{
				return;
			}

			// Allocate space for the piece tracker table.
			if (!m_UpdatePieces.Alloc((msg->UpdateSize + PIECE_BYTES - 1) / PIECE_BYTES))
			{
				return;
			}

			m_ServerVersion = msg->ServerVersion;
			memcpy(&m_UpdateSignature, &msg->UpdateSignature, sizeof(Signature));

			m_Status.Bytes = 0;
			m_Status.Total = m_UpdateData.Size;

			m_IsUpdating = true;
		}
		else if (header->Type == MessageType::SERVER_UPDATE_PIECE)
		{
			if (len != sizeof(ServerUpdatePieceMessage))
			{
				return;
			}

			if (!m_IsUpdating)
			{
				return;
			}

			ServerUpdatePieceMessage *msg = (ServerUpdatePieceMessage *)BUF;

			// Check that the version matches.
			if (m_ServerVersion != msg->ServerVersion)
			{
				return;
			}

			// Verify that the tokens match.
			if (msg->ClientToken != m_ClientToken || msg->ServerToken != m_ServerToken)
			{
				return;
			}

			// Verify that the message piece size is valid.
			if (msg->PieceSize > PIECE_BYTES)
			{
				return;
			}

			// Verify that the message contains the full piece data.
			if (len != (sizeof(ServerUpdatePieceMessage) + msg->PieceSize))
			{
				return;
			}

			// Verify that the position is on a piece boundry and something we actually requested.
			if ((msg->PiecePos % PIECE_BYTES) != 0)
			{
				return;
			}

			// Verify that the data doesn't write outside the buffer.
			if (msg->PiecePos + msg->PieceSize > m_UpdateData.Size)
			{
				return;
			}

			// Verify the piece position.
			uint32 idx = msg->PiecePos / PIECE_BYTES;
			if (idx >= m_UpdatePieces.Size)
			{
				return;
			}

			// Verify that we need the piece.
			if (m_UpdatePieces.Ptr[idx])
			{
				return;
			}

			// Validate that the piece size aligns with how we request data.
			if (idx < m_UpdatePieces.Size - 1)
			{
				if (msg->PieceSize != PIECE_BYTES)
				{
					return;
				}
			}

			memcpy(m_UpdateData.Ptr + msg->PiecePos, BUF + sizeof(ServerUpdatePieceMessage), msg->PieceSize);
			m_UpdatePieces.Ptr[idx] = 1;
			m_Status.Bytes += msg->PieceSize;
		}
		else if (header->Type == MessageType::SERVER_UPDATE_TOKEN)
		{
			if (len != sizeof(ServerUpdateTokenMessage))
			{
				return;
			}

			ServerUpdateTokenMessage *msg = (ServerUpdateTokenMessage *)BUF;
			if (msg->ClientToken != m_ClientToken)
			{
				return;
			}

			m_ServerToken = msg->ServerToken;

			// If we need an update, send the command to send the update.
			if (m_ClientVersion != m_LocalVersion)
			{
				// Send update begin request to server
				ClientUpdateBeginMessage begin_update = {};
				begin_update.Header.Type = MessageType::CLIENT_UPDATE_BEGIN;
				begin_update.Header.Version = m_ClientVersion;
				begin_update.ClientVersion = m_ClientVersion;
				begin_update.ClientToken = m_ClientToken;
				begin_update.ServerToken = m_ServerToken;
				m_Socket->Send(&begin_update, sizeof(begin_update), m_Host);
			}
		}
	}
}

void Client::Reset()
{
	m_UpdateData.Free();
	m_UpdatePieces.Free();

	m_IsFinished = false;
	m_IsUpdating = false;

	m_ClientToken = 0;
	m_ServerToken = 0;
	m_ServerVersion = 0;

	m_UpdateIdx = 0;
}

