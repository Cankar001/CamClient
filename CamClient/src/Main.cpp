#include <iostream>

#include "Client.h"

#if 0
#include "PythonCamera.h"

int main(int argc, char *argv[])
{
	PythonCamera::Camera c(false, 1280, 720);
	c.Stream();
	c.Show();
	c.ReleaseStream();

	return 0;
}
#endif

int main(int argc, char *argv[])
{
	ClientConfig config;
	config.ServerIP = "127.0.0.1";
	config.Port = 45645;

	Client c(config);

	// start all worker threads
	c.Run();

	return 0;
}