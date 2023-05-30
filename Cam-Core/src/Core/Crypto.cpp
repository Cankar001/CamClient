#include "Crypto.h"

#ifdef CAM_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsCrypto.h"
#elif CAM_PLATFORM_LINUX
#include "Platform/Linux/LinuxCrypto.h"
#endif

namespace Core
{
	Crypto *Crypto::Create()
	{
#ifdef CAM_PLATFORM_WINDOWS
		return new WindowsCrypto();
#elif CAM_PLATFORM_LINUX
		return new LinuxCrypto();
#endif
	}
}

