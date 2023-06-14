#pragma once

#include <Cam-Core.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>

#include <opencv2/opencv.hpp>

struct ServerConfig
{
	/// <summary>
	/// The IP address of the server.
	/// </summary>
	std::string ServerIP;
	
	/// <summary>
	/// The port, at which the server should listen.
	/// </summary>
	uint16 Port;

	/// <summary>
	/// The duration in minutes of each camera feed to be kept in memory for saving to disk after something happened.
	/// </summary>
	uint32 VideoBackupDuration;
};

struct ClientEntry
{
	Core::addr_t Address;
	Core::RingBuffer<cv::Mat> Frames;
	std::string FrameTitle;

	ClientEntry(uint32 frame_size)
		: Frames(Core::RingBuffer<cv::Mat>(frame_size))
	{
		Address = {};
	}

	inline bool operator==(const ClientEntry &other) const
	{
		return Address.Value == other.Address.Value && Frames.Size() == other.Frames.Size();
	}

	inline bool operator!=(const ClientEntry &other) const
	{
		return !(*this == other);
	}

	friend bool operator==(ClientEntry &lhs, const ClientEntry &rhs)
	{
		return lhs.Address.Value == rhs.Address.Value && lhs.Frames.Size() == rhs.Frames.Size();
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

	std::vector<ClientEntry> m_Clients;
	std::thread m_FramePreviewThread;
};

