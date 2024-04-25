#include <iostream>

#include "DisplayClient.h"

int main(int argc, char *argv[])
{
	Core::Init();

	DisplayClientConfig config;
	DisplayClient display(config);

	display.Run();

	Core::Shutdown();
	return 0;
}

