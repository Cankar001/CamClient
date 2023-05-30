#include "Core/FileSystem.h"

#ifdef CAM_PLATFORM_LINUX

namespace Core
{
	int64 FileSystem::Seek(const std::string &filePath, int64 offset, int64 origin)
	{
		return 0;
	}

	int64 FileSystem::Size(const std::string &filePath)
	{
		return 0;
	}

	uint32 FileSystem::ReadTextFile(const std::string &filePath, std::string *out_str)
	{
		return 0;
	}

	bool FileSystem::WriteTextFile(const std::string &filePath, const std::string &str)
	{
		return false;
	}

	bool FileSystem::WriteFile(const std::string &filePath, void *src, uint32 bytes)
	{
		return false;
	}

	bool FileSystem::ReadFile(const std::string &filePath, void *dst, uint32 *outSize)
	{
		return false;
	}

	uint32 FileSystem::Print(const std::string &filePath, const char *fmt, ...)
	{
		return 0;
	}

	bool FileSystem::SetCurrentWorkingDirectory(const std::string &directory)
	{
		return false;
	}

	bool FileSystem::GetCurrentWorkingDirectory(std::string *out_directory)
	{
		return false;
	}

	bool FileSystem::DirectoryExists(const std::string &filePath) const
	{
		return false;
	}

	bool FileSystem::FileExists(const std::string &filePath) const
	{
		return false;
	}

	bool FileSystem::RemoveFile(const std::string &filePath) const
	{
		return false;
	}

	bool FileSystem::RemoveDirectoy(const std::string &filePath) const
	{
		return false;
	}
}

#endif // CAM_PLATFORM_LINUX

