#include <iostream>
#include "Updater.h"

/// <summary>
/// The client starts automatically, if the camera is turned on.
/// It first asks the server, if an update is avaiable and updates the binaries, if an update is received.
/// After that it runs as a daemon and listens for server instructions, but also starts the actual camera client.
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns></returns>
int main(int argc, char *argv[])
{
	Core::FileSystem *sys = Core::FileSystem::Create();
	std::string cwd = "";
	sys->SetCurrentWorkingDirectory("../../../");
	sys->GetCurrentWorkingDirectory(&cwd);
	std::cout << "Current CWD: " << cwd.c_str() << std::endl;

	UpdateConfig config;
	config.UpdateTargetPath = "../CamClient";
	config.ServerIP = "127.0.0.1";
	config.Port = 44200;
	Updater updater(config);

	// First ask the server, if update is avaiable
	if (updater.IsUpdateAvail())
	{
		updater.Run();
	}

	// TODO: Start the camera client


	delete sys;
	return 0;
}

