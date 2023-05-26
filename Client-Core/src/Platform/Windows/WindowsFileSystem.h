#pragma once

#ifdef CAM_PLATFORM_WINDOWS

#include "Core/FileSystem.h"

#include <Windows.h>

namespace Core
{
	class WindowsFileSystem : public FileSystem
	{
	public:

		WindowsFileSystem();
		~WindowsFileSystem();

		virtual bool Open(const std::string &filePath, const std::string &writeMode) override;
		virtual void Close() override;

		virtual bool IsValid() const override;
		virtual int64 Seek(int64 offset, int64 origin) override;
		virtual int64 Size() override;

		virtual uint32 Read(void *dst, uint32 bytes) override;
		virtual uint32 Write(void const *src, uint32 bytes) override;

		virtual uint32 ReadTextFile(std::string *out_str) override;
		virtual bool WriteTextFile(const std::string &str) override;

		virtual uint32 Print(const char *fmt, ...) override;

		virtual bool SetCurrentWorkingDirectory(const std::string &directory) override;
		virtual bool GetCurrentWorkingDirectory(std::string *out_directory) override;

	private:

		HANDLE m_Handle;
	};
}

#endif // CAM_PLATFORM_WINDOWS

