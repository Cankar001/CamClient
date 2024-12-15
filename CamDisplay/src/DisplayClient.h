#pragma once

#include <Cam-Core.h>

struct DisplayClientConfig
{
	std::string ServerIP;
	uint16 Port;
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

