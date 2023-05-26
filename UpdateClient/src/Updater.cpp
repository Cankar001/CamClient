#include "Updater.h"

Updater::Updater(const UpdateConfig &config)
	: m_Config(config)
{
	m_Socket = Core::Socket::Create();
}

Updater::~Updater()
{
	delete m_Socket;
	m_Socket = nullptr;
}

bool Updater::IsUpdateAvail() const
{
	return false;
}

void Updater::Run()
{
}

