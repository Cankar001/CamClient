#pragma once

#include <string>
#include <functional>

namespace Core
{
	enum class FileSystemWatcherAction
	{
		None = 0,
		Added,
		Removed,
		Modified,
		Renamed,
	};

	struct FileSystemWatcherContext
	{
		/// <summary>
		/// Is set, if a file was renamed before.
		/// </summary>
		std::string OldName;

		/// <summary>
		/// The type of the target file, telling what happened.
		/// </summary>
		FileSystemWatcherAction Action;

		/// <summary>
		/// The target file, which has changed.
		/// </summary>
		std::string FilePath;
	};

	using FileSystemWatcherCallbackFn = std::function<void(const FileSystemWatcherContext&)>;

	class FileSystemWatcher
	{
	public:

		static void Start(const std::string &filePath, const FileSystemWatcherCallbackFn &callback);
		static void Stop();

		static void SetWatchPath(const std::string &filePath);
		static void DisableWatchUntilNextAction();

	private:

		static unsigned long Watch(void *param);
	};
}

