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
	config.TargetBinaryPath = "../CamClient/bin/Debug-windows-x86_64/CamClient";
	config.PublicKeyPath = "../CamClient/public_key.key";
	config.PrivateKeyPath = "../CamClient/private_key.key";
	config.SignaturePath = "../CamClient/signature.sig";

	Server *s = new Server(config);
	if (!s->LoadUpdateFile())
	{
		std::cerr << "Could not load the update!" << std::endl;

		delete s;
		delete sys;
		return -1;
	}

	s->Run();

	delete s;
	delete sys;
	return 0;
}

