#include <iostream>

#include "Server.h"

int main(int argc, char *argv[])
{
	Core::FileSystem::Get()->SetCurrentWorkingDirectory("../../../");

	std::string cwd = "";
	bool cwd_success = Core::FileSystem::Get()->GetCurrentWorkingDirectory(&cwd);
	if (cwd_success)
	{
		std::cout << "Current CWD: " << cwd.c_str() << std::endl;
	}

	// Get current start directory from command line arguments
	if (argc > 1)
	{
		// First argument will be the start directory
		std::string selected_cwd = std::string(argv[1]);
		cwd_success = Core::FileSystem::Get()->SetCurrentWorkingDirectory(selected_cwd);
		if (cwd_success)
		{
			cwd_success = Core::FileSystem::Get()->GetCurrentWorkingDirectory(&cwd);
			if (cwd_success)
			{
				std::cout << "Changed CWD to: " << cwd.c_str() << std::endl;	
			}
		}
	}

	ServerConfig config;
	config.ServerIP = "127.0.0.1";
	config.Port = 45645;
	config.VideoBackupDuration = 5;

	Server s(config);
	s.StartFramePreviews();
	s.Run();

	return 0;
}

