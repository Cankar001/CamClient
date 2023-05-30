#include "Socket.h"

#ifdef CAM_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsSocket.h"
#elif CAM_PLATFORM_LINUX
#include "Platform/Linux/LinuxSocket.h"
#endif

namespace Core
{
	Socket *Socket::Create()
	{
#ifdef CAM_PLATFORM_WINDOWS
		return new WindowsSocket();
#elif CAM_PLATFORM_LINUX
		return new LinuxSocket();
#endif
	}
}

