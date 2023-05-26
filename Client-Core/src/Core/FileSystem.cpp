#include "FileSystem.h"

#ifdef CAM_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsFileSystem.h"
#elif CAM_PLATFORM_LINUX
#include "Platform/Linux/LinuxFileSystem.h"
#endif

namespace Core
{
	FileSystem *FileSystem::Create()
	{
#ifdef CAM_PLATFORM_WINDOWS
		return new WindowsFileSystem();
#elif CAM_PLATFORM_LINUX
		return new LinuxFileSystem();
#endif
	}
}

