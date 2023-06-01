#pragma once

#include <Cam-Core.h>
#include <string>

struct ServerConfig
{
	std::string ServerIP;
	uint16 Port;
};

class Server
{
public:

	Server(const ServerConfig &config);
	~Server();

	void Run();

private:

	bool Step();

	bool OnClientConnected(Core::addr_t &clientAddr, Byte *message, int32 addrLen);
	bool OnClientDisconnected(Core::addr_t &clientAddr, Byte *message, int32 addrLen);
	bool OnClientFrame(Core::addr_t &clientAddr, Byte *message, int32 addrLen);

private:

	ServerConfig m_Config;
	Core::Socket *m_Socket = nullptr;

	uint32 m_Version;
};

