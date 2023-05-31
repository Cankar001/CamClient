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

	/// <summary>
	/// the path to the public key
	/// </summary>
	std::string PublicKeyPath;

	/// <summary>
	/// The path to the private key
	/// </summary>
	std::string PrivateKeyPath;

	/// <summary>
	/// The path to the signature
	/// </summary>
	std::string SignaturePath;
};

class Server
{
public:

	Server(const ServerConfig &config);
	~Server();

	/// <summary>
	/// Runs the server.
	/// </summary>
	void Run();

	/// <summary>
	/// Loads up the directory and assembles the update.
	/// </summary>
	/// <returns>Returns true, if the update was built successfully and if the server is ready to run.</returns>
	bool LoadUpdateFile(bool forceDeleteSignature = false);

	/// <summary>
	/// Watches the update path and automatically sends the new update to all clients
	/// </summary>
	void StartFileWatcher();

private:

	bool Step();

private:

	ServerConfig m_Config;
	Core::Socket *m_Socket = nullptr;
	Core::Crypto *m_Crypto = nullptr;
	Core::IPTable *m_IPTable = nullptr;
	Core::Clients *m_Clients = nullptr;

	Core::Crypto::key_t m_PublicKey = {};

	// Update data.
	int64 m_LastUpdateCheckMS;
	int64 m_LastUpdateWriteMS;
	uint32 m_LocalVersion;
	Signature m_UpdateSignature;
	Core::FileSystemBuffer m_UpdateFile;
};

