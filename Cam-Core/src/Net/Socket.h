#pragma once

#include "Core/Core.h"
#include <string>

namespace Core
{
	// IPV4 host port/pair.
	union addr_t {
		uint64 Value;

		struct {
			uint32 Host;
			uint32 Port;
		};
	};

	class Socket
	{
	public:

		virtual ~Socket() {}

		virtual bool Open(bool is_client = false, const std::string &ip = "", uint16 port = 0) = 0;
		virtual void Close() = 0;

		virtual bool Bind(uint16 port) = 0;

		virtual int32 Recv(void *dst, int32 dst_bytes, addr_t *addr) = 0;
		virtual int32 Send(void const *src, int32 src_bytes, addr_t addr) = 0;

		virtual int32 SendLarge(void const *src, int32 src_bytes, addr_t addr) = 0;
		virtual int32 RecvLarge(void *dst, int32 dst_bytes, addr_t *addr) = 0;

		virtual bool SetNonBlocking(bool enabled) = 0;
		virtual addr_t Lookup(const std::string &host, uint16 port) = 0;

		static Socket *Create();
	};
}

