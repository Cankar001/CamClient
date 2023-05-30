#include "Core/FileSystemWatcher.h"

#ifdef CAM_PLATFORM_LINUX

namespace Core
{
	static bool s_Watching = false;
	static bool s_IgnoreNextChange = false;
	static FileSystemWatcherCallbackFn s_Callback;
	static std::string s_WatchPath = "";
	static std::string s_OldName = "";

	void FileSystemWatcher::Start(const std::string &filePath, const FileSystemWatcherCallbackFn &callback)
	{
		s_WatchPath = filePath;
		s_Callback = callback;
		s_Watching = true;


	}

	void FileSystemWatcher::Stop()
	{
		s_Watching = false;


	}

	void FileSystemWatcher::SetWatchPath(const std::string &filePath)
	{
		s_WatchPath = filePath;

		Stop();
		Start(filePath, s_Callback);
	}

	void FileSystemWatcher::DisableWatchUntilNextAction()
	{
		s_IgnoreNextChange = true;
	}

	unsigned long FileSystemWatcher::Watch(void *param)
	{
		return 0ul;
	}
}

#endif // CAM_PLATFORM_LINUX

