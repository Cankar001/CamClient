#pragma once

#include <vector>
#include <string>

namespace Core
{
	struct ZipFile
	{
		std::string Name;
		std::string Path; //!< @deprecated
		void *Buffer;
		size_t BufferSize;
	};

	class ZipArchive
	{
	public:

		ZipArchive();
		virtual ~ZipArchive();

		std::vector<ZipFile> Load(const std::string &inFilepath);
		bool Store(const std::vector<ZipFile> &inZipfiles, const std::string &inFilepath);

	private:


	};
}
