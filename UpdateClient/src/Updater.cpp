#include "Updater.h"

#include <iostream>

Updater::Updater(const UpdateConfig &config)
	: m_Config(config)
{
	m_Socket = Core::Socket::Create();
	m_FileSystem = Core::FileSystem::Create();

	if (!m_Socket->Open())
	{
		std::cerr << "Socket could not be opened!" << std::endl;
	}

	if (!m_Socket->SetNonBlocking(true))
	{
		std::cerr << "Socket could not be set to non-blocking!" << std::endl;
	}

	m_Host = m_Socket->Lookup(m_Config.ServerIP, m_Config.Port);
}

Updater::~Updater()
{
	delete m_FileSystem;
	m_FileSystem = nullptr;

	delete m_Socket;
	m_Socket = nullptr;
}

bool Updater::IsUpdateAvail() const
{
	// First load the local client version
	std::string local_path = m_Config.UpdateTargetPath + "/src/CamVersion.h";
	m_FileSystem->Open(local_path, "r");

	std::cout << "Trying to read " << local_path.c_str() << std::endl;
	Core::FileSystemBuffer buffer;
	if (!m_FileSystem->Read(&buffer, sizeof(buffer)))
	{
		std::cerr << "Could not read local version file!" << std::endl;
		return false;
	}

	std::cout << buffer.Data << std::endl;

	m_FileSystem->Close();

	// Then request the latest client version from the server

	// and finally compare both versions

	return false;
}

void Updater::Run()
{
	for (;;)
	{
		static Byte BUF[65536];

		Core::addr_t addr;
		int32 len = m_Socket->Recv(BUF, sizeof(BUF), &addr);
		if (len < 0)
		{
			break;
		}

		// ignore all messages from unknown senders
		if (addr.Value != m_Host.Value)
		{
			continue;
		}


	}
}

