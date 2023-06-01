#pragma once

#include <Cam-Core.h>

struct DisplayClientConfig
{

};

class DisplayClient
{
public:

	DisplayClient(const DisplayClientConfig &config);
	~DisplayClient();

	void Run();

private:

	DisplayClientConfig m_Config;
};

