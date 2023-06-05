#include <iostream>

#include "Server.h"

int main(int argc, char *argv[])
{
	ServerConfig config;
	config.ServerIP = "127.0.0.1";
	config.Port = 45645;
	config.VideoBackupDuration = 5;

	Server s(config);
	s.StartFramePreviews();
	s.Run();

	return 0;
}

