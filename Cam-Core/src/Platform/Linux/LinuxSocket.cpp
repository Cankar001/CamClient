#include "LinuxSocket.h"

#ifdef CAM_PLATFORM_LINUX

#include <assert.h>
#include <netdb.h>

namespace Core
{
	LinuxSocket::LinuxSocket()
	{
		m_Socket = -1;
	}

	LinuxSocket::~LinuxSocket()
	{
		Close();
	}
	
	bool LinuxSocket::Open(bool is_client, const std::string &ip, uint16 port)
	{
		Close();

		int32 opt = 1;

		// Creating socket file descriptor
		if ((m_Socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			return false;
		}

		// Forcefully attaching socket to the port 8080
		if (setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
		{
			return false;
		}

		if (is_client)
		{
			struct sockaddr_in serv_addr;
			int status;

			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port = htons(port);

			// Convert IPv4 and IPv6 addresses from text to binary form
			if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0)
			{
				return false;
			}

			if ((status = connect(m_Socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
			{
				return false;
			}
		}

		return true;
	}
	
	void LinuxSocket::Close()
	{
		close(m_Socket);
		shutdown(m_Socket, SHUT_RDWR);

		m_Socket = -1;
	}
	
	bool LinuxSocket::Bind(uint16 port)
	{
		struct sockaddr_in address;
		int32 addrlen = sizeof(address);
		int32 new_socket;

		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);

		// Forcefully attaching socket to the port
		if (bind(m_Socket, (struct sockaddr *)&address, sizeof(address)) < 0)
		{
			return false;
		}

		if (listen(m_Socket, 3) < 0)
		{
			return false;
		}

		if ((new_socket = accept(m_Socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
		{
			return false;
		}

		return true;
	}
	
	int32 LinuxSocket::Recv(void *dst, int32 dst_bytes, addr_t *addr)
	{
		return read(m_Socket, dst, dst_bytes);
	}
	
	int32 LinuxSocket::Send(void const *src, int32 src_bytes, addr_t addr)
	{
		return send(m_Socket, src, src_bytes, 0);
	}
	
	bool LinuxSocket::SetNonBlocking(bool enabled)
	{
		return fcntl(m_Socket, F_SETFL, O_NONBLOCK);
	}
	
	addr_t LinuxSocket::Lookup(const std::string &host, uint16 port)
	{
		assert(host.size() > 0);
		struct gaicb *reqs[1];
		char hbuf[NI_MAXHOST];
		struct addrinfo *res;

		addr_t addr = {};

		reqs[0] = malloc(sizeof(*reqs[0]));
		memset(reqs[0], 0, sizeof(*reqs[0]));
		reqs[0]->ar_name = host.c_str();

		int32 ret = getaddrinfo_a(GAI_WAIT, reqs, 1, NULL);
		if (ret != 0)
		{
			free(reqs[0]);
			reqs[0] = NULL;
			return {};
		}

		ret = gai_error(reqs[0]);
		if (ret == 0)
		{
			res = reqs[0]->ar_result;

			ret = getnameinfo(res->ai_addr, res->ai_addrlen, hbuf, sizeof(hbuf), NULL, 0, NI_NUMERICHOST);
			if (ret != 0)
			{
				free(reqs[0]);
				reqs[0] = NULL;
				return {};
			}

			addr.Host = hbuf;
			addr.Port = port;
		}

		free(reqs[0]);
		reqs[0] = NULL;
		return addr;
	}
}

#endif // CAM_PLATFORM_LINUX

