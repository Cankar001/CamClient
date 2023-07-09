#include "Core/FileSystem.h"

#ifdef CAM_PLATFORM_LINUX

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

namespace Core
{
	int64 FileSystem::Seek(const std::string &filePath, int64 offset, int64 origin)
	{
		return 0;
	}

	int64 FileSystem::Size(const std::string &filePath)
	{
		FILE *f = fopen(filePath.c_str(), "r");
		if (f)
		{
			fseek(f, 0, SEEK_END);
			int64 length = ftell(f);
			fseek(f, 0, SEEK_SET);

			fclose(f);
			return length;
		}

		return 0;
	}

	uint32 FileSystem::ReadTextFile(const std::string &filePath, std::string *out_str)
	{
		if (!out_str)
		{
			std::cout << "Needed valid pointer to the result string!" << std::endl;
			return 0;
		}

		char *buffer = 0;
		uint32 length = 0;
		FILE *f = fopen(filePath.c_str(), "r");

		if (f)
		{
			fseek(f, 0, SEEK_END);
			length = ftell(f);
			fseek(f, 0, SEEK_SET);

			buffer = (char*)malloc(length);
			fread(buffer, 1, length, f);
			fclose(f);

			if (buffer)
			{
				*out_str = std::string(buffer);
				free(buffer);
				return length;
			}
		}

		std::cout << "Could not open file " << filePath.c_str() << std::endl;
		return 0;
	}

	bool FileSystem::WriteTextFile(const std::string &filePath, const std::string &str)
	{
		return WriteFile(filePath, (void*)&str[0], (uint32)str.size());
	}

	bool FileSystem::WriteFile(const std::string &filePath, void *src, uint32 bytes)
	{
		FILE *f = fopen(filePath.c_str(), "wb");
		if (f)
		{

			fclose(f);
		}

		return false;
	}

	Byte *FileSystem::ReadFile(const std::string &filePath, uint32 *outSize)
	{
		return nullptr;
	}

	uint32 FileSystem::Print(const std::string &filePath, const char *fmt, ...)
	{
		return 0;
	}

	bool FileSystem::SetCurrentWorkingDirectory(const std::string &directory)
	{
		if (chdir(directory.c_str()) == -1)
		{
			char buffer[256];
    		char *errorMsg = strerror_r(errno, buffer, 256);
			printf("Error %s", errorMsg);
			return false;
		}

		return true;
	}

	bool FileSystem::GetCurrentWorkingDirectory(std::string *out_directory)
	{
   		char cwd[PATH_MAX];
   		if (getcwd(cwd, sizeof(cwd)) != NULL)
		{
			*out_directory = std::string(cwd);
			return true;
		}

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

