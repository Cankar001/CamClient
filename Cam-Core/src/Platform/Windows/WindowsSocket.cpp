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
	
	bool WindowsSocket::Open(bool is_client, const std::string &ip, uint16 port)
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
			std::cerr << "Socket is invalid" << std::endl;
			return -1;
		}

		if (addr.Value == 0)
		{
			std::cerr << "Host is invalid" << std::endl;
			return 0;
		}

		SOCKADDR_IN si = {};
		si.sin_family = AF_INET;
		si.sin_addr.s_addr = addr.Host;
		si.sin_port = (uint16)addr.Port;

		int32 result = sendto(m_Socket, (CHAR const *)src, src_bytes, 0, (SOCKADDR *)&si, sizeof(si));
		if (result == SOCKET_ERROR)
		{
			int32 errorCode = WSAGetLastError();
			std::cerr << "Socket error: " << errorCode << std::endl;
		}

		return result;
	}

	int32 WindowsSocket::SendLarge(void const *src, int32 src_bytes, addr_t addr)
	{
		int32 send_pos = 0;
		int32 buffer_size = 256;
		int32 bytes_left = src_bytes;
		int32 n = -1;
		
		SOCKADDR_IN si = {};
		si.sin_family = AF_INET;
		si.sin_addr.s_addr = addr.Host;
		si.sin_port = (uint16)addr.Port;

		while (send_pos < src_bytes)
		{
			int32 chunk_size = bytes_left > buffer_size ? buffer_size : bytes_left;
			n = sendto(m_Socket, (CHAR const *)src + send_pos, chunk_size, 0, (SOCKADDR *)&si, sizeof(si));
			
			if (-1 == n)
			{
				break;
			}

			if (n != chunk_size)
			{
				// if less bytes were send, increase/decrease only by that amount
				chunk_size = n;
			}

			send_pos += chunk_size;
			bytes_left -= chunk_size;
		}

		return send_pos == src_bytes ? send_pos : -1;
	}

	int32 WindowsSocket::RecvLarge(void *dst, int32 dst_bytes, addr_t *addr)
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
		int32 buffer_size = 256;
		int32 read_pos = 0;
		int32 bytes_received = 0;
		
		while (bytes_received != dst_bytes)
		{
			if (read_pos >= dst_bytes)
			{
				break;
			}
	
			uint32 chunk_size = dst_bytes > buffer_size ? buffer_size : dst_bytes;
			bytes_received = recvfrom(m_Socket, (char*)dst + read_pos, chunk_size, 0, (struct sockaddr *)&si, &sil);
			
			if (bytes_received == -1)
			{
				break;
			}
			
			if (chunk_size != bytes_received)
			{
				chunk_size = bytes_received;
			}

			read_pos += chunk_size;

			addr->Host = si.sin_addr.s_addr;
			addr->Port = si.sin_port;
		}

		return bytes_received == dst_bytes ? bytes_received : -1;
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

