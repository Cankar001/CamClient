#include "LinuxSocket.h"

#ifdef CAM_PLATFORM_LINUX

namespace Core
{
	LinuxSocket::LinuxSocket()
	{
	}

	LinuxSocket::~LinuxSocket()
	{
	}
	
	bool LinuxSocket::Open()
	{
		return false;
	}
	
	void LinuxSocket::Close()
	{
	}
	
	bool LinuxSocket::Bind(uint16 port)
	{
		return false;
	}
	
	int32 LinuxSocket::Recv(void *dst, int32 dst_bytes, addr_t *addr)
	{
		return int32();
	}
	
	int32 LinuxSocket::Send(void const *src, int32 src_bytes, addr_t addr)
	{
		return int32();
	}
	
	bool LinuxSocket::SetNonBlocking(bool enabled)
	{
		return false;
	}
	
	addr_t LinuxSocket::Lookup(const std::string &host, uint16 port)
	{
		return addr_t();
	}
}

#endif // CAM_PLATFORM_LINUX

