#pragma once

#include "Core.h"
#include "Singleton.h"
#include <string>

namespace Core
{
	class FileSystem : public Singleton<FileSystem>
	{
	public:

		int64 Seek(const std::string &filePath, int64 offset, int64 origin);
		int64 Size(const std::string &filePath);

		uint32 ReadTextFile(const std::string &filePath, std::string *out_str);
		bool WriteTextFile(const std::string &filePath, const std::string &str);

		bool WriteFile(const std::string &filePath, void *src, uint32 bytes);
		Byte *ReadFile(const std::string &filePath, uint32 *outSize);

		uint32 Print(const std::string &filePath, const char *fmt, ...);

		bool SetCurrentWorkingDirectory(const std::string &directory);
		bool GetCurrentWorkingDirectory(std::string *out_directory);

		bool DirectoryExists(const std::string &filePath) const;
		bool FileExists(const std::string &filePath) const;
		
		bool RemoveFile(const std::string &filePath) const;
		bool RemoveDirectoy(const std::string &filePath) const;
	};

	struct FileSystemBuffer
	{
		Byte *Data;
		uint32 Size;

		FileSystemBuffer()
			: Data(nullptr), Size(0)
		{
		}

		inline ~FileSystemBuffer()
		{
			Free();
		}

		void Alloc(uint32 size)
		{
			Data = new Byte[size];
			Size = size;
		}

		inline void Free()
		{
			free(Data);
			Data = nullptr;
			Size = 0;
		}
	};
}

