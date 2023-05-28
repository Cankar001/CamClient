#include "Client.h"

#include <iostream>

#include "Utils/Utils.h"

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
	uint32 local_version = Core::utils::GetLocalVersion(m_FileSystem, m_Config.UpdateTargetPath);
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
			std::cout << "Starting CamClient..." << std::endl;

			m_Status.Code = ClientStatusCode::NONE;
		}

		MessageLoop();

		// Process updates.
		auto now_ms = Core::QueryMS();
		UpdateProgress(now_ms, m_Host);

		static uint32 last_bytes;
		if (last_bytes != m_Status.Bytes)
		{
			last_bytes = m_Status.Bytes;
		}

		// Limit the update rate.
		Core::SleepMS(10);
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

void Client::UpdateProgress(int64 now_ms, Core::addr_t addr)
{
	if (m_IsFinished)
	{
		return;
	}

	if (!m_IsUpdating)
	{
		if (now_ms - m_LastUpdateMS >= 1000)
		{
			m_LastUpdateMS = now_ms;
			m_ClientToken = m_Crypto->GenToken();

			// Send update begin request to server
			ClientUpdateBeginMessage begin_update = {};
			begin_update.Header.Type = MessageType::CLIENT_UPDATE_BEGIN;
			begin_update.Header.Version = m_ClientVersion;
			begin_update.ClientVersion = m_ClientVersion;
			begin_update.ClientToken = m_ClientToken;
			begin_update.ServerToken = m_ServerToken;
			m_Socket->Send(&begin_update, sizeof(begin_update), m_Host);
			m_Status.Code = ClientStatusCode::NONE;
		}

		return;
	}

	// We are updating
	if (m_UpdateIdx >= m_UpdatePieces.Size)
	{
		uint32 crc = Core::Crc32(m_UpdateData.Ptr, m_UpdateData.Size);
		if (crc == m_ServerVersion)
		{
			if (m_Crypto->TestSignature(m_UpdateSignature.Data, sizeof(m_UpdateSignature.Data), m_UpdateData.Ptr, m_UpdateData.Size, m_Config.PublicKey, sizeof(m_Config.PublicKey)))
			{
				if (m_FileSystem->WriteFile(m_Config.UpdateTargetPath + "/update.zip", m_UpdateData.Ptr, m_UpdateData.Size))
				{
					m_IsFinished = true;
					m_Status.Code = ClientStatusCode::UP_TO_DATE;
					return;
				}
				else
				{
					m_Status.Code = ClientStatusCode::BAD_WRITE;
				}
			}
			else
			{
				m_Status.Code = ClientStatusCode::BAD_SIG;
			}
		}
		else
		{
			m_Status.Code = ClientStatusCode::BAD_CRC;
		}

		Reset();
		return;
	}

	// Update in progress, request pieces.
	if (now_ms - m_LastPieceMS >= 100)
	{
		m_LastPieceMS = now_ms;

		ClientUpdatePieceMessage msg = {};
		msg.Header.Version = m_LocalVersion;
		msg.Header.Type = MessageType::CLIENT_UPDATE_PIECE;
		msg.ClientToken = m_ClientToken;
		msg.ServerToken = m_ServerToken;
		msg.ServerVersion = m_ServerVersion;

		bool found_missing = false;
		uint32 end_idx = 0;
		uint32 num_pieces = 0;

		// Request missing piecees.
		for (uint32 idx = m_UpdateIdx, end = m_UpdatePieces.Size; idx < end; ++idx)
		{
			if (!m_UpdatePieces.Ptr[idx])
			{
				// Keep track of the last valid idx.
				if (!found_missing)
				{
					found_missing = true;
					end_idx = idx;
				}

				msg.PiecePos = idx * PIECE_BYTES;
				m_Socket->Send(&msg, sizeof(msg), m_Host);

				num_pieces += 1;
				if (num_pieces > MAX_REQUESTS)
				{
					// Don't send too many.
					break;
				}
			}
		}

		if (end_idx > m_UpdateIdx)
		{
			m_UpdateIdx = end_idx;
		}

		if (!found_missing)
		{
			m_UpdateIdx = m_UpdatePieces.Size;
		}
	}
}

