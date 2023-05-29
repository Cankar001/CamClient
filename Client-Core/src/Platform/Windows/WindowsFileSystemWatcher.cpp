#include "Core/FileSystemWatcher.h"

#ifdef CAM_PLATFORM_WINDOWS

#include "Core/Core.h"

#include <assert.h>
#include <Windows.h>

namespace Core
{
	static bool s_Watching = false;
	static bool s_IgnoreNextChange = false;
	static HANDLE s_WatcherThread;
	static std::string s_WatchPath = "";
	static std::string s_OldName = "";
	static FileSystemWatcherCallbackFn s_Callback;

	void FileSystemWatcher::Start(const std::string &filePath, const FileSystemWatcherCallbackFn &callback)
	{
		DWORD threadId;
		s_Callback = callback;
		s_WatchPath = filePath;
		s_WatcherThread = CreateThread(NULL, 0, Watch, 0, 0, &threadId);
		assert(s_WatcherThread != NULL);
		SetThreadDescription(s_WatcherThread, L"CamVisionFileSystemWatcher");

		s_Watching = true;
	}

	void FileSystemWatcher::Stop()
	{
		s_Watching = false;
		TerminateThread(s_WatcherThread, 0);
		CloseHandle(s_WatcherThread);
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
		char buffer[2048];
		DWORD bytesReturned;
		BOOL result = true;

		HANDLE dirHandle = CreateFileA(s_WatchPath.c_str(), GENERIC_READ | FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		if (dirHandle == INVALID_HANDLE_VALUE)
		{
			return 0;
		}

		OVERLAPPED pollingOverlap;
		pollingOverlap.OffsetHigh = 0;
		pollingOverlap.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

		while (s_Watching && result)
		{
			result = ReadDirectoryChangesW(dirHandle, &buffer, sizeof(buffer), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE, &bytesReturned, &pollingOverlap, NULL);

			WaitForSingleObject(pollingOverlap.hEvent, INFINITE);

			if (s_IgnoreNextChange)
			{
				s_IgnoreNextChange = false;
				result = false;
				continue;
			}

			FILE_NOTIFY_INFORMATION *pNotify;
			int32 offset = 0;

			do
			{
				pNotify = (FILE_NOTIFY_INFORMATION *)((char *)buffer + offset);
				uint32 fileNameLength = pNotify->FileNameLength / (sizeof(wchar_t));

				std::wstring tmpW = std::wstring(pNotify->FileName, fileNameLength);

#pragma warning( push )
#pragma warning( disable : 4244 ) 
				std::string tmp = std::string(tmpW.begin(), tmpW.end());
#pragma warning( pop )

				FileSystemWatcherAction action = FileSystemWatcherAction::None;

				switch (pNotify->Action)
				{
				case FILE_ACTION_ADDED:
				{
					action = FileSystemWatcherAction::Added;
					break;
				}

				case FILE_ACTION_REMOVED:
				{
					action = FileSystemWatcherAction::Removed;
					break;
				}

				case FILE_ACTION_MODIFIED:
				{
					action = FileSystemWatcherAction::Modified;
					break;
				}

				case FILE_ACTION_RENAMED_OLD_NAME:
				{
					s_OldName = tmp;
					break;
				}

				case FILE_ACTION_RENAMED_NEW_NAME:
				{
					action = FileSystemWatcherAction::Renamed;
					break;
				}
				}

				if (pNotify->Action != FILE_ACTION_RENAMED_OLD_NAME)
				{
					FileSystemWatcherContext context = {};
					context.Action = action;
					context.FilePath = tmp;
					context.OldName = s_OldName;
					s_Callback(context);
				}

				offset += pNotify->NextEntryOffset;

			} while (pNotify->NextEntryOffset);
		}

		CloseHandle(dirHandle);
		return 0;
	}
}

#endif // CAM_PLATFORM_WINDOWS

