#pragma once

#ifdef CAM_PLATFORM_LINUX

#include "Core/FileSystem.h"

namespace Core
{
	class LinuxFileSystem : public FileSystem
	{
	public:

		LinuxFileSystem();
		~LinuxFileSystem();

		virtual bool Open(const std::string &filePath, const std::string &writeMode) override;
		virtual void Close() override;

		virtual bool IsValid() const override;
		virtual int64 Seek(int64 offset, int64 origin) override;
		virtual int64 Size() override;

		virtual uint32 Read(void *dst, uint32 bytes) override;
		virtual uint32 Write(void const *src, uint32 bytes) override;

		virtual uint32 ReadTextFile(std::string *out_str) override;
		virtual bool WriteTextFile(const std::string &str) override;

		virtual bool WriteFile(const std::string &filePath, void *src, uint32 bytes) override;
		virtual bool ReadFile(const std::string &filePath, void *dst, uint32 *outSize) override;

		virtual uint32 Print(const char *fmt, ...) override;

		virtual bool SetCurrentWorkingDirectory(const std::string &directory) override;
		virtual bool GetCurrentWorkingDirectory(std::string *out_directory) override;

	private:


	};
}

#endif // CAM_PLATFORM_LINUX

