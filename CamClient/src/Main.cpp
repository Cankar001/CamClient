#include <iostream>

#include "Camera.h"

int main(int argc, char *argv[])
{
	Camera c(false, 1280, 720);
	c.Stream();
	c.Show();
	c.ReleaseStream();

	return 0;
}

