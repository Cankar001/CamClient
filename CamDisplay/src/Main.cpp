#include <iostream>

#include "DisplayClient.h"

int main(int argc, char *argv[])
{
	Core::Init();

	DisplayClientConfig config;
	config.ServerIP = "127.0.0.1";
	config.Port = 45645;
	DisplayClient display(config);

	display.Run();

	Core::Shutdown();
	return 0;
}

