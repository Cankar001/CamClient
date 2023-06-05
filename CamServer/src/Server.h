#pragma once

#include <Cam-Core.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>

#include <opencv2/opencv.hpp>

struct ServerConfig
{
	std::string ServerIP;
	uint16 Port;
};

struct ClientEntry
{
	Core::addr_t Address;
	std::vector<cv::Mat> Frames;
	std::string FrameTitle;

	inline bool operator==(const ClientEntry &other) const
	{
		return Address.Value == other.Address.Value && Frames.size() == other.Frames.size();
	}

	inline bool operator!=(const ClientEntry &other) const
	{
		return !(*this == other);
	}

	friend bool operator==(ClientEntry &lhs, const ClientEntry &rhs)
	{
		return lhs.Address.Value == rhs.Address.Value && lhs.Frames.size() == rhs.Frames.size();
	}

	friend bool operator!=(ClientEntry &lhs, const ClientEntry &rhs)
	{
		return !(lhs == rhs);
	}

	friend bool operator==(ClientEntry &lhs, const Core::addr_t &address)
	{
		return lhs.Address.Value == address.Value;
	}

	friend bool operator!=(ClientEntry &lhs, const Core::addr_t &address)
	{
		return !(lhs == address);
	}
};

class Server
{
public:

	Server(const ServerConfig &config);
	~Server();

	void Run();
	void StartFramePreviews();

private:

	bool Step();

	bool OnClientConnected(Core::addr_t &clientAddr, Byte *message, int32 addrLen);
	bool OnClientDisconnected(Core::addr_t &clientAddr, Byte *message, int32 addrLen);
	bool OnClientFrame(Core::addr_t &clientAddr, Byte *message, int32 addrLen);

	void FramePreview();

private:

	ServerConfig m_Config;
	Core::Socket *m_Socket = nullptr;

	uint32 m_Version;
	bool m_Running = true;

	std::vector<ClientEntry> m_Frames;
	std::thread m_FramePreviewThread;
};

