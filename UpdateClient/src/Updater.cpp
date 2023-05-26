#include "Updater.h"

#include <iostream>

namespace utils
{
	static bool HasMacroInText(const std::string &str, const std::string &macro)
	{
		size_t pos = str.find("#define " + macro);
		if (pos == std::string::npos)
		{
			return false;
		}

		return true;
	}

	static std::string GetMacroFromText(const std::string &str, const std::string &macro)
	{
		std::string macro_search_text = "#define " + macro;
		size_t pos = str.find(macro_search_text);
		if (pos == std::string::npos)
		{
			return "";
		}

		std::string result = str;
		result = result.substr(pos + macro_search_text.size() + 1);

		pos = result.find("\n");
		if (pos == std::string::npos)
		{
			return result;
		}

		result = result.substr(0, pos);
		return result;
	}
}

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
	if (!m_FileSystem->Open(local_path, "r"))
	{
		std::cerr << "Could not open file " << local_path.c_str() << std::endl;
		return false;
	}

	std::string version_content = "";
	if (!m_FileSystem->ReadTextFile(&version_content))
	{
		std::cerr << "Could not read version file!" << std::endl;
		return false;
	}

	std::string local_version_str = utils::GetMacroFromText(version_content, "CAM_VERSION");
	uint32 local_version = std::stoi(local_version_str);
	std::cout << "Local version: " << local_version << std::endl;

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

