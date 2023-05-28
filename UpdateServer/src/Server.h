#pragma once

#include <Client-Core.h>
#include <string>

#include "Message.h"

struct ServerConfig
{
	/// <summary>
	/// Contains the path to the root path of the source, to read the version.
	/// </summary>
	std::string TargetUpdatePath;

	/// <summary>
	/// Contains the path to the binaries from the source, which should be shipped to the clients.
	/// </summary>
	std::string TargetBinaryPath;

	/// <summary>
	/// The IP v4 address of the server
	/// </summary>
	std::string ServerIP;

	/// <summary>
	/// The port of the server
	/// </summary>
	uint16 ServerPort;
};

class Server
{
public:

	Server(const ServerConfig &config);
	~Server();

	void Run();

private:

	bool Step();

private:

	ServerConfig m_Config;
	Core::Socket *m_Socket = nullptr;
	Core::FileSystem *m_FileSystem = nullptr;
	Core::Crypto *m_Crypto = nullptr;
	Core::IPTable *m_IPTable = nullptr;
	Core::Clients *m_Clients = nullptr;

	// Refreshes the update data if needed.
	bool UpdateRefresh(int64 now_ms);

	// Update data.
	int64 m_LastUpdateCheckMS;
	int64 m_LastUpdateWriteMS;
	uint32 m_LocalVersion;
	uint32 m_UpdateVersion;
	Signature m_UpdateSignature;
	Core::FileSystemBuffer m_UpdateFile;
};

