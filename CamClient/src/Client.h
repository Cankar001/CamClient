#pragma once

#include <Cam-Core.h>
#include <string>
#include <thread>

#include "Camera.h"

struct ClientConfig
{
	std::string ServerIP;
	uint16 Port;
};

class Client
{
public:

	Client(const ClientConfig &config);
	~Client();

	/// <summary>
	/// Runs the client, is responsible to start all worker threads.
	/// </summary>
	void Run();

	/// <summary>
	/// Handles the communication with the server.
	/// </summary>
	void NetworkLoop();

	/// <summary>
	/// Handles receiving the frames from the connected camera.
	/// </summary>
	void CameraLoop();

private:

	bool OnConnectionAccepted(Byte *message, uint32 length);
	bool OnConnectionClosed(Byte *message, uint32 length);
	bool OnFrameResponse(Byte *message, uint32 length);

private:

	ClientConfig m_Config;
	Core::Socket *m_Socket = nullptr;
	Core::addr_t m_Host;
	
	uint32 m_Version;
	bool m_Running = true;
	Camera m_Camera;

	std::thread m_NetworkThread;
	std::thread m_CameraThread;
};