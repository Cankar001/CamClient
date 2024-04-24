#include <iostream>

#include "Server.h"

int main(int argc, char *argv[])
{
	// Set the current working directory
	Core::FileSystem::Get()->SetCurrentWorkingDirectory("../../../");

	ServerConfig config;
	config.ServerIP = "127.0.0.1";
	config.ServerPort = 44200;
	config.TargetSourcePath = "../CamClient";
	config.TargetBinaryPath = "../CamClient/bin/Debug-windows-x86_64/CamClient";
	config.PublicKeyPath = "../CamClient/public_key.key";
	config.PrivateKeyPath = "../CamClient/private_key.key";
	config.SignaturePath = "../CamClient/signature.sig";

	Server *s = new Server(config);
	if (!s->LoadUpdateFile(true))
	{
		std::cerr << "Could not load the update!" << std::endl;

		delete s;
		return -1;
	}

	s->StartFileWatcher();

	s->Run();

	delete s;
	return 0;
}

