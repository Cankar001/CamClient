#include <iostream>

#include "Camera.h"

int main(int argc, char *argv[])
{
	Camera c(true);
	c.Stream();

	// Pseudo loop to keep the program running until user decides to shutdown
	while (c.IsStreaming())
	{
	}

	c.ReleaseStream();

	return 0;
}

