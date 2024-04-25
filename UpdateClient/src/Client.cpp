#include "Client.h"

#include <iostream>
#include <assert.h>

#include "Utils/Utils.h"
#include "Core/Log.h"

static uint32 MAX_RECV_ATTEMPTS = 250;

Client::Client(const ClientConfig &config)
	: m_Config(config)
{
	m_Socket = Core::Socket::Create();
	m_Crypto = Core::Crypto::Create();

	std::string cwd = "";
	Core::FileSystem::Get()->GetCurrentWorkingDirectory(&cwd);

	LoadLocalVersion();

	CAM_LOG_INFO("===================== CONFIG ===================================");
	CAM_LOG_INFO("IP                    : {}", config.ServerIP);
	CAM_LOG_INFO("Port                  : {}", config.Port);
	CAM_LOG_INFO("Update binary path    : {}", config.UpdateBinaryPath);
	CAM_LOG_INFO("Update source path    : {}", config.UpdateTargetPath);
	CAM_LOG_INFO("Current Client version: {}", m_LocalVersion);
	CAM_LOG_INFO("Current CWD           : {}", cwd);
	CAM_LOG_INFO("================================================================");

	if (!m_Socket->Open(true, m_Config.ServerIP, m_Config.Port))
	{
		CAM_LOG_ERROR("Socket could not be opened!");
	}

	if (!m_Socket->SetNonBlocking(true))
	{
		CAM_LOG_ERROR("Socket could not be set to non-blocking!");
	}

	m_Host = m_Socket->Lookup(m_Config.ServerIP, m_Config.Port);
	Reset();
}

Client::~Client()
{
	delete m_Crypto;
	m_Crypto = nullptr;

	delete m_Socket;
	m_Socket = nullptr;
}

void Client::RequestServerVersion()
{
	// Then request the latest client version from the server
	CAM_LOG_DEBUG("Sending server version request...");
	ClientWantsVersionMessage message = {};
	message.LocalVersion = m_LocalVersion;
	message.ClientVersion = 0;
	message.Header.Type = MessageType::CLIENT_REQUEST_VERSION;
	message.Header.Version = m_LocalVersion;
	uint32 bytes_sent = m_Socket->Send(&message, sizeof(message), m_Host);
	assert(bytes_sent == sizeof(message));

	m_Status.Code = ClientStatusCode::NONE;
}

void Client::Run()
{
	// When running for the first time, request the server version
	RequestServerVersion();

	for (;;)
	{
		if (m_CurrentRecvAttempt >= MAX_RECV_ATTEMPTS)
		{
			m_CurrentRecvAttempt = 0;
			break;
		}

		if (m_Status.Code == ClientStatusCode::UP_TO_DATE)
		{
			CAM_DEBUG_BREAK;
#if 0
			if (ExtractUpdate(m_Config.UpdateBinaryPath + "/update.zip"))
			{
				// TODO: start CamClient application
				CAM_LOG_INFO("Starting CamClient...");

				// Update local version
				m_LocalVersion = Core::utils::GetLocalVersion(m_Config.UpdateTargetPath);

				m_Status.Code = ClientStatusCode::NONE;
			}
			else
			{
				CAM_LOG_ERROR("Could not extract the archive!");
			}
#endif
		}
		else if (m_Status.Code == ClientStatusCode::BAD_SIG)
		{
			CAM_LOG_ERROR("Bad Signature from last update.");
		}
		else if (m_Status.Code == ClientStatusCode::BAD_WRITE)
		{
			CAM_LOG_ERROR("Bad Write from last update.");
		}

		MessageLoop();

		// Process updates.
		int64 now_ms = Core::QueryMS();
		UpdateProgress(now_ms, m_Host);

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
			++m_CurrentRecvAttempt;
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
				CAM_LOG_ERROR("Unexpected network message size!");
				return;
			}

			ServerVersionInfoMessage *msg = (ServerVersionInfoMessage *)BUF;
			CAM_LOG_DEBUG("Received new server version: {}", msg->Version);
			if (msg->Version != m_LocalVersion)
			{
				// The versions are different, we need an update
				m_Status.Code = ClientStatusCode::NEEDS_UPDATE;
				m_ClientVersion = msg->Version;

				// Copy the public key once
				m_Config.PublicKey.Size = msg->PublicKey.Size;
				memcpy(m_Config.PublicKey.Data, msg->PublicKey.Data, msg->PublicKey.Size);
				
				// reset the update state, so that in the next update the update will start
				m_IsFinished = false;
				CAM_LOG_WARN("Requiring update...");
			}
			else
			{
				// The versions are the same, we have the latest version
				m_Status.Code = ClientStatusCode::UP_TO_DATE;
				CAM_LOG_INFO("Binaries are up-to-date. No action required.");
			}
		}
		else if (header->Type == MessageType::SERVER_UPDATE_BEGIN)
		{
			if (len != sizeof(ServerUpdateBeginMessage))
			{
				CAM_LOG_ERROR("Received wrong package size");
				return;
			}

			ServerUpdateBeginMessage *msg = (ServerUpdateBeginMessage *)BUF;
			
			// Verify that the update size is reasonable (<200MB).
			if (msg->UpdateSize == 0 || msg->UpdateSize >= (200 * 1024 * 1024))
			{
				CAM_LOG_ERROR("Update size was very unrealistic! Size: {}", msg->UpdateSize);
				return;
			}

			// Allocate space for update data.
			if (!m_UpdateData.Alloc(msg->UpdateSize))
			{
				CAM_LOG_ERROR("Could not allocated enough space for update!");
				return;
			}

			// Allocate space for the piece tracker table.
			if (!m_UpdatePieces.Alloc((msg->UpdateSize + PIECE_BYTES - 1) / PIECE_BYTES))
			{
				CAM_LOG_ERROR("Could not allocated enough space for update!");
				return;
			}

			memcpy(&m_UpdateSignature, &msg->UpdateSignature, sizeof(Signature));
			CAM_LOG_DEBUG("Received update begin request, total size: {}", msg->UpdateSize);

			m_Status.Bytes = 0;
			m_Status.Total = m_UpdateData.Size;

			m_IsUpdating = true;
			m_IsFinished = false;
		}
		else if (header->Type == MessageType::SERVER_UPDATE_PIECE)
		{
			if (!m_IsUpdating)
			{
				CAM_LOG_ERROR("Invalid state! The update is not in progress.");
				return;
			}

			ServerUpdatePieceMessage *msg = (ServerUpdatePieceMessage *)BUF;

			// Verify that the tokens match.
			if (msg->ClientToken != m_ClientToken || msg->ServerToken != m_ServerToken)
			{
				CAM_LOG_ERROR("Client or server token did not match!");
				return;
			}

			// Verify that the message piece size is valid.
			if (msg->PieceSize > PIECE_BYTES)
			{
				CAM_LOG_ERROR("Unexpected message piece size! Expected {0}, but got {1}", PIECE_BYTES, msg->PieceSize);
				return;
			}

			// Verify that the message contains the full piece data.
			if (len != (sizeof(ServerUpdatePieceMessage) + msg->PieceSize))
			{
				CAM_LOG_ERROR("The network package has not the expected size!");
				return;
			}

			// Verify that the position is on a piece boundry and something we actually requested.
			if ((msg->PiecePos % PIECE_BYTES) != 0)
			{
				// TODO: Log error
				return;
			}

			// Verify that the data doesn't write outside the buffer.
			if (msg->PiecePos + msg->PieceSize > m_UpdateData.Size)
			{
				CAM_LOG_ERROR("The piece pos offset is larger than the update size!");
				return;
			}

			// Verify the piece position.
			uint32 idx = msg->PiecePos / PIECE_BYTES;
			if (idx >= m_UpdatePieces.Size)
			{
				CAM_LOG_ERROR("The piece index is larger than the update size!");
				return;
			}

			// Verify that we need the piece.
			if (m_UpdatePieces.Ptr[idx])
			{
				// TODO: Log error
				return;
			}

			// Validate that the piece size aligns with how we request data.
			if (idx < m_UpdatePieces.Size - 1)
			{
				if (msg->PieceSize != PIECE_BYTES)
				{
					// TODO: Log error
					return;
				}
			}

			// Copy the update piece into the target buffer.
			memcpy(m_UpdateData.Ptr + msg->PiecePos, BUF + sizeof(ServerUpdatePieceMessage), msg->PieceSize);
			m_UpdatePieces.Ptr[idx] = 1;
			m_Status.Bytes += msg->PieceSize;
		}
		else if (header->Type == MessageType::SERVER_UPDATE_TOKEN)
		{
			if (len != sizeof(ServerUpdateTokenMessage))
			{
				// TODO: Log error
				return;
			}

			ServerUpdateTokenMessage *msg = (ServerUpdateTokenMessage *)BUF;
			if (msg->ClientToken != m_ClientToken)
			{
				CAM_LOG_ERROR("Client tokens did not match!");
				return;
			}

			CAM_LOG_INFO("Received new server token: {}", msg->ServerToken);
			m_ServerToken = msg->ServerToken;
		}
	}
}

#if 0
bool Client::ExtractUpdate(const std::string &zipPath)
{
	mz_zip_archive zip_archive;

	CAM_LOG_DEBUG("Extracting archive {}...", zipPath);

	mz_bool status = mz_zip_reader_init_file(&zip_archive, zipPath.c_str(), 0);
	if (!status)
	{
		CAM_LOG_ERROR("Could not open the ZIP file!");
		return false;
	}

	for (uint32 i = 0; i < mz_zip_reader_get_num_files(&zip_archive); ++i)
	{
		mz_zip_archive_file_stat file_stat;
		if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
		{
			CAM_LOG_ERROR("Could not make the file stats");

			if (!mz_zip_reader_end(&zip_archive))
			{
				CAM_LOG_ERROR("Could not close the zip reader");
			}

			return false;
		}

		std::string file_name = file_stat.m_filename;
		std::string file_name_on_disk = m_Config.UpdateBinaryPath + "/" + file_name;
		std::string comment = file_stat.m_comment;
		uint64 uncompressed_size = file_stat.m_uncomp_size;
		uint64 compressed_size = file_stat.m_comp_size;
		mz_bool is_dir = mz_zip_reader_is_file_a_directory(&zip_archive, i);
		CAM_LOG_DEBUG("Extracting file {}", file_name);

		Byte *p = (Byte*)mz_zip_reader_extract_file_to_heap(&zip_archive, file_name.c_str(), &uncompressed_size, 0);

		if (!Core::FileSystem::Get()->WriteFile(file_name_on_disk, p, (uint32)uncompressed_size))
		{
			CAM_LOG_ERROR("Could not write the file {}", file_name_on_disk);
			return false;
		}

		delete[] p;
		p = nullptr;
	}

	if (!mz_zip_reader_end(&zip_archive))
	{
		CAM_LOG_ERROR("Could not close the zip reader");
		return false;
	}

	CAM_LOG_INFO("Archive {} extracted successfully.", zipPath);
	return true;
}
#endif

bool Client::LoadLocalVersion()
{
	// First load the local client version
	uint32 local_version = Core::utils::GetLocalVersion(m_Config.UpdateTargetPath);
	if (!local_version)
	{
		return false;
	}

	m_LocalVersion = local_version;
	return true;
}

void Client::Reset()
{
	m_UpdateData.Free();
	m_UpdatePieces.Free();

	m_IsFinished = true;
	m_IsUpdating = false;

	m_ClientToken = 0;
	m_ServerToken = 0;

	m_UpdateIdx = 0;
}

void Client::UpdateProgress(int64 now_ms, Core::addr_t addr)
{
	if (m_IsFinished)
	{
		CAM_LOG_INFO("Finished updating progress.");
		return;
	}

	if (!m_IsUpdating)
	{
		if (now_ms - m_LastUpdateMS >= 1000)
		{
			m_LastUpdateMS = now_ms;
			m_ClientToken = m_Crypto->GenToken();

			// Send update begin request to server
			CAM_LOG_DEBUG("Sending update begin request...");
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
	CAM_LOG_DEBUG("Loading update: {0} / {1}", m_UpdateIdx, m_UpdatePieces.Size);
	if (m_UpdateIdx >= m_UpdatePieces.Size)
	{
		if (m_Crypto->TestSignature(m_UpdateSignature.Data, SIG_BYTES, m_UpdateData.Ptr, m_UpdateData.Size, m_Config.PublicKey.Data, m_Config.PublicKey.Size))
		{
			CAM_LOG_DEBUG("Writing file {}", (m_Config.UpdateBinaryPath + "/update.zip"));
			if (Core::FileSystem::Get()->WriteFile(m_Config.UpdateBinaryPath + "/update.zip", m_UpdateData.Ptr, m_UpdateData.Size))
			{
				m_IsFinished = true;
				m_Status.Code = ClientStatusCode::UP_TO_DATE;
				CAM_LOG_DEBUG("File {} written successfully.", (m_Config.UpdateBinaryPath + "/update.zip"));
				return;
			}
			else
			{
				m_Status.Code = ClientStatusCode::BAD_WRITE;
				CAM_DEBUG_BREAK;
			}
		}
		else
		{
			m_Status.Code = ClientStatusCode::BAD_SIG;
			CAM_DEBUG_BREAK;
		}

		Reset();
		return;
	}

	// Update in progress, request pieces.
	if (now_ms - m_LastPieceMS >= 100)
	{
		m_LastPieceMS = now_ms;

		CAM_LOG_DEBUG("Sending update piece request...");
		ClientUpdatePieceMessage msg = {};
		msg.Header.Version = m_LocalVersion;
		msg.Header.Type = MessageType::CLIENT_UPDATE_PIECE;
		msg.ClientToken = m_ClientToken;
		msg.ServerToken = m_ServerToken;

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
			CAM_LOG_INFO("Requested all update pieces successfully.");
		}
	}
}

