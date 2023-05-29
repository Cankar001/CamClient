#pragma once

#include "Core.h"
#include <string>

namespace Core
{
	class FileSystem
	{
	public:

		virtual ~FileSystem() {}

		virtual bool Open(const std::string &filePath, const std::string &writeMode) = 0;
		virtual void Close() = 0;

		virtual bool IsValid() const = 0;
		virtual int64 Seek(int64 offset, int64 origin) = 0;
		virtual int64 Size() = 0;

		virtual uint32 Read(void *dst, uint32 bytes) = 0;
		virtual uint32 Write(void const *src, uint32 bytes) = 0;

		virtual uint32 ReadTextFile(std::string *out_str) = 0;
		virtual bool WriteTextFile(const std::string &str) = 0;

		virtual bool WriteFile(const std::string &filePath, void *src, uint32 bytes) = 0;
		virtual bool ReadFile(const std::string &filePath, void *dst, uint32 *outSize) = 0;

		virtual uint32 Print(const char *fmt, ...) = 0;

		virtual bool SetCurrentWorkingDirectory(const std::string &directory) = 0;
		virtual bool GetCurrentWorkingDirectory(std::string *out_directory) = 0;

		virtual bool DirectoryExists(const std::string &filePath) const = 0;
		virtual bool FileExists(const std::string &filePath) const = 0;
		
		virtual bool RemoveFile(const std::string &filePath) const = 0;
		virtual bool RemoveDirectoy(const std::string &filePath) const = 0;

		static FileSystem *Create();
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

