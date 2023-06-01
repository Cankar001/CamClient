#include <iostream>

#include "PythonCamera.h"

int main(int argc, char *argv[])
{
	PythonCamera::Camera c(false, 1280, 720);
	c.Stream();
	c.Show();
	c.ReleaseStream();

	return 0;
}

