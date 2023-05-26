#pragma once

#include <Client-Core.h>

#include <string>

struct UpdateConfig
{
	/// <summary>
	/// The path, where the received files from the update server should be stored to
	/// </summary>
	std::string UpdateTargetPath;

	/// <summary>
	/// The update server ip
	/// </summary>
	std::string ServerIP;

	/// <summary>
	/// The update server port
	/// </summary>
	uint16 Port;
};

class Updater
{
public:

	Updater(const UpdateConfig &config);
	~Updater();

	bool IsUpdateAvail() const;
	void Run();

private:

	UpdateConfig m_Config;
	Core::Socket *m_Socket = nullptr;
};