#include "WindowsFileSystem.h"

#ifdef CAM_PLATFORM_WINDOWS

#include <assert.h>

namespace Core
{
	WindowsFileSystem::WindowsFileSystem()
		: m_Handle(INVALID_HANDLE_VALUE)
	{
	}

	WindowsFileSystem::~WindowsFileSystem()
	{
		Close();
	}
	
	bool WindowsFileSystem::Open(const std::string &filePath, const std::string &writeMode)
	{
		DWORD access = 0;
		DWORD disposition = 0;

		if (writeMode == "r")
		{
			access = GENERIC_READ;
			disposition = OPEN_EXISTING;
		}
		else if (writeMode == "w")
		{
			access = GENERIC_WRITE;
			disposition = CREATE_NEW;
		}

		m_Handle = CreateFileA(filePath.c_str(), access, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, disposition, FILE_ATTRIBUTE_NORMAL, NULL);
		return (m_Handle != INVALID_HANDLE_VALUE);
	}
	
	void WindowsFileSystem::Close()
	{
		if (m_Handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_Handle);
			m_Handle = INVALID_HANDLE_VALUE;
		}
	}
	
	bool WindowsFileSystem::IsValid() const
	{
		return (m_Handle != INVALID_HANDLE_VALUE);
	}
	
	int64 WindowsFileSystem::Seek(int64 offset, int64 origin)
	{
		LARGE_INTEGER loff, lpos;
		loff.QuadPart = offset;
		return m_Handle && SetFilePointerEx(m_Handle, loff, &lpos, (DWORD)origin) ? lpos.QuadPart : 0;
	}
	
	int64 WindowsFileSystem::Size()
	{
		LARGE_INTEGER result;
		return m_Handle && GetFileSizeEx(m_Handle, &result) ? result.QuadPart : 0;
	}
	
	uint32 WindowsFileSystem::Read(void *dst, uint32 bytes)
	{
		assert(dst);
		assert(bytes);

		DWORD result;
		return ReadFile(m_Handle, dst, bytes, &result, 0) ? result : 0;
	}
	
	uint32 WindowsFileSystem::Write(void const *src, uint32 bytes)
	{
		assert(src);
		assert(bytes);


		DWORD result;
		return WriteFile(m_Handle, src, bytes, &result, 0) ? result : 0;
	}
	
	uint32 WindowsFileSystem::Print(const char *fmt, ...)
	{
		assert(fmt);

		va_list args;
		va_start(args, fmt);

		CHAR buf[4096];
		int32 res = vsnprintf(buf, sizeof(buf), fmt, args);
		if (res >= sizeof(buf))
		{
			// truncation
			res = sizeof(buf) - 1;
		}

		va_end(args);

		if (res <= 0)
		{
			return 0;
		}

		return Write(buf, (uint32)res);
	}
	
	bool WindowsFileSystem::SetCurrentWorkingDirectory(const std::string &directory)
	{
		return ::SetCurrentDirectoryA(directory.c_str());
	}

	bool WindowsFileSystem::GetCurrentWorkingDirectory(std::string *out_directory)
	{
		CHAR buffer[MAX_PATH];
		DWORD dwRet;

		if (!out_directory)
		{
			return false;
		}

		dwRet = ::GetCurrentDirectoryA(MAX_PATH, buffer);

		if (dwRet == 0)
		{
			return false;
		}

		if (dwRet >= MAX_PATH)
		{
			return false;
		}

		buffer[dwRet] = '\0';
		*out_directory = std::string(buffer);
		return true;
	}
}

#endif // CAM_PLATFORM_WINDOWS

