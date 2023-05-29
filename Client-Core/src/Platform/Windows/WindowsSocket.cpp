#include "WindowsSocket.h"

#ifdef CAM_PLATFORM_WINDOWS

#include <iostream>
#include <assert.h>
#include <WS2tcpip.h>

namespace Core
{
	WindowsSocket::WindowsSocket()
		: m_Socket(INVALID), m_IsWsaInitialized(false)
	{
		WSADATA wsadata;
		if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
		{
			return;
		}

		m_IsWsaInitialized = true;
	}

	WindowsSocket::~WindowsSocket()
	{
		Close();

		if (m_IsWsaInitialized)
		{
			WSACleanup();
		}
	}
	
	bool WindowsSocket::Open()
	{
		Close();
		m_Socket = socket(AF_INET, SOCK_DGRAM, 0);
		return (m_Socket != INVALID);
	}
	
	void WindowsSocket::Close()
	{
		if (m_Socket != INVALID)
		{
			closesocket(m_Socket);
			m_Socket = INVALID;
		}
	}
	
	bool WindowsSocket::Bind(uint16 port)
	{
		struct sockaddr_in si = {};
		si.sin_family = AF_INET;
		si.sin_addr.s_addr = INADDR_ANY;
		si.sin_port = htons(port);

		return (::bind(m_Socket, (struct sockaddr *)&si, sizeof(si)) == 0);
	}
	
	int32 WindowsSocket::Recv(void *dst, int32 dst_bytes, addr_t *addr)
	{
		assert(dst);
		assert(dst_bytes);
		assert(addr);

		if (m_Socket == INVALID)
		{
			return -1;
		}

		struct sockaddr_in si = {};
		int32 sil = sizeof(si);
		int32 len = recvfrom(m_Socket, (char *)dst, dst_bytes, 0, (struct sockaddr *)&si, &sil);

		addr->Host = si.sin_addr.s_addr;
		addr->Port = si.sin_port;

		return len;
	}
	
	int32 WindowsSocket::Send(void const *src, int32 src_bytes, addr_t addr)
	{
		assert(src);
		assert(src_bytes);

		if (m_Socket == INVALID)
		{
			return -1;
		}

		if (addr.Value == 0)
		{
			return 0;
		}

		SOCKADDR_IN si = {};
		si.sin_family = AF_INET;
		si.sin_addr.s_addr = addr.Host;
		si.sin_port = (uint16)addr.Port;

		int32 result = sendto(m_Socket, (CHAR const *)src, src_bytes, 0, (SOCKADDR *)&si, sizeof(si));
		if (result == SOCKET_ERROR)
		{
			std::cerr << "Socket error" << std::endl;
		}

		return result;
	}
	
	bool WindowsSocket::SetNonBlocking(bool enabled)
	{
		if (m_Socket == INVALID)
		{
			return false;
		}

		u_long val = enabled;
		return (ioctlsocket(m_Socket, FIONBIO, &val) == 0);
	}
	
	addr_t WindowsSocket::Lookup(const std::string &host, uint16 port)
	{
		assert(host.size() > 0);

		addr_t addr = {};

		struct addrinfo hints = {};
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_family = AF_INET;

		struct addrinfo *res = nullptr;
		if (getaddrinfo(host.c_str(), 0, &hints, &res) == 0)
		{
			auto sai = (struct sockaddr_in *)res->ai_addr;
			addr.Host = sai->sin_addr.s_addr;
			addr.Port = htons(port);
			freeaddrinfo(res);
		}

		return addr;
	}
}

#endif // CAM_PLATFORM_WINDOWS

