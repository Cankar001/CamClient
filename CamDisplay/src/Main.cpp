#include <iostream>

#include "DisplayClient.h"

int main(int argc, char *argv[])
{
	DisplayClientConfig config;
	DisplayClient display(config);

	display.Run();

	return 0;
}

