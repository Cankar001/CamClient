#include <iostream>

#include "Server.h"

int main(int argc, char *argv[])
{
	// Set the current working directory
	Core::FileSystem *sys = Core::FileSystem::Create();
	std::string cwd = "";
	sys->SetCurrentWorkingDirectory("../../../");
	sys->GetCurrentWorkingDirectory(&cwd);
	std::cout << "Current CWD: " << cwd.c_str() << std::endl;

	ServerConfig config;
	config.ServerIP = "127.0.0.1";
	config.ServerPort = 44200;
	config.TargetUpdatePath = "../CamClient";
	config.TargetBinaryPath = "../CamClient/bin";

	Server s(config);
	s.Run();

	delete sys;
	return 0;
}

