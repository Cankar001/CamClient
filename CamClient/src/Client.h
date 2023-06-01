#pragma once

#include <Cam-Core.h>
#include <string>
#include <thread>
#include <vector>

#include "Camera.h"
#include "Messages.h"

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
	/// Frees all camera resources and joins all worker threads.
	/// </summary>
	void Release();

	/// <summary>
	/// Tells, if the client is currently running.
	/// </summary>
	/// <returns>Returns true, if all worker threads are running.</returns>
	bool IsRunning() const { return m_Running; }

	/// <summary>
	/// Runs the client, is responsible to start all worker threads.
	/// </summary>
	/// <param name="shouldShowFrames">Determines, if the main thread should show all current camera frames.</param>
	void Run(bool shouldShowFrames = true);

	/// <summary>
	/// Handles the communication with the server.
	/// </summary>
	void NetworkLoop();

	/// <summary>
	/// Handles receiving the frames from the connected camera.
	/// </summary>
	void CameraLoop();

	/// <summary>
	/// Shows all current frames of the camera.
	/// </summary>
	void Show();

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
	bool m_NetworkThreadFinished = false;
	bool m_SentConnectionCloseRequest = false;
	bool m_ConnectedToServer = false;
	Camera m_Camera;

	std::thread m_NetworkThread;
	std::thread m_CameraThread;

	std::vector<FrameData> m_Frames;
};