#pragma once

#include <Client-Core.h>

#include <string>

#include "Message.h"

struct ClientConfig
{
	/// <summary>
	/// The path, where the received files from the update server should be stored to
	/// </summary>
	std::string UpdateTargetPath;

	/// <summary>
	/// 
	/// </summary>
	std::string UpdateBinaryPath;

	/// <summary>
	/// The update server ip
	/// </summary>
	std::string ServerIP;

	/// <summary>
	/// The update server port
	/// </summary>
	uint16 Port;

	/// <summary>
	/// The public key, generated when starting the update client
	/// </summary>
	Core::Crypto::key_t PublicKey = {};
};

enum class ClientStatusCode
{
	/// <summary>
	/// This is the idle state, where the client just listens for updates, which may be sent by the update server
	/// </summary>
	NONE = 0,

	/// <summary>
	/// The client received a different server version and therefore needs a local update
	/// </summary>
	NEEDS_UPDATE,

	/// <summary>
	/// The client received the same version from the server and will start the CamClient.
	/// </summary>
	UP_TO_DATE,
	
	/// <summary>
	/// The CRC did not match.
	/// </summary>
	BAD_CRC,
	
	/// <summary>
	/// The signature is missing or not generated.
	/// </summary>
	BAD_SIG,
	
	/// <summary>
	/// An error occurred, when writing the update to disk.
	/// </summary>
	BAD_WRITE
};

struct ClientStatus
{
	uint32 Bytes;
	uint32 Total;
	ClientStatusCode Code;
};

class Client
{
public:

	Client(const ClientConfig &config);
	~Client();

	void RequestServerVersion();
	void Run();
	void Reset();

	void UpdateProgress(int64 now_ms, Core::addr_t addr);

private:

	void MessageLoop();

private:

	static constexpr uint32 MAX_REQUESTS = 32;

	ClientConfig m_Config;
	Core::Socket *m_Socket = nullptr;
	Core::Crypto *m_Crypto = nullptr;
	Core::addr_t m_Host;

	Core::Buffer m_UpdateData;
	Core::Buffer m_UpdatePieces;

	int64 m_LastUpdateMS;
	int64 m_LastPieceMS;
	uint64 m_ClientToken;
	uint64 m_ServerToken;
	uint32 m_ClientVersion;
	uint32 m_LocalVersion;
	uint32 m_ServerVersion;
	ClientStatus m_Status;
	bool m_IsFinished = true;
	bool m_IsUpdating = false;
	uint32 m_UpdateIdx;
	Signature m_UpdateSignature;
};

