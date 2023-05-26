#include "LinuxFileSystem.h"

#ifdef CAM_PLATFORM_LINUX

namespace Core
{
	LinuxFileSystem::LinuxFileSystem()
	{
	}

	LinuxFileSystem::~LinuxFileSystem()
	{
	}
	
	bool LinuxFileSystem::Open(const std::string &filePath, const std::string &writeMode)
	{
		return false;
	}
	
	void LinuxFileSystem::Close()
	{
	}
	
	bool LinuxFileSystem::IsValid() const
	{
		return false;
	}
	
	int64 LinuxFileSystem::Seek(int64 offset, int64 origin)
	{
		return int64();
	}
	
	int64 LinuxFileSystem::Size()
	{
		return int64();
	}
	
	uint32 LinuxFileSystem::Read(void *dst, uint32 bytes)
	{
		return uint32();
	}
	
	uint32 LinuxFileSystem::Write(void const *src, uint32 bytes)
	{
		return uint32();
	}

	uint32 LinuxFileSystem::ReadTextFile(std::string *out_str)
	{
		return uint32();
	}

	bool LinuxFileSystem::WriteTextFile(const std::string &str)
	{
		return false;
	}
	
	uint32 LinuxFileSystem::Print(const char *fmt, ...)
	{
		return uint32();
	}

	bool LinuxFileSystem::SetCurrentWorkingDirectory(const std::string &directory)
	{
	}

	bool LinuxFileSystem::GetCurrentWorkingDirectory(std::string *out_directory)
	{
		return false;
	}
}

#endif // CAM_PLATFORM_LINUX

