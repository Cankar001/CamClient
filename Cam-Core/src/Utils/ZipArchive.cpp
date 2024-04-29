#include "ZipArchive.h"

#include <miniz/miniz.h>
#include <iostream>

#include "Core/FileSystem.h"

namespace Core
{
	ZipArchive::ZipArchive()
	{
	}

	ZipArchive::~ZipArchive()
	{
	}

	std::vector<ZipFile> ZipArchive::Load(const std::string &inFilepath)
	{
		std::vector<ZipFile> result;

		uint32_t zip_size = 0;
		Byte* zip_data = FileSystem::Get()->ReadFile(inFilepath, &zip_size);

		mz_zip_archive archive;
		memset(&archive, 0, sizeof(mz_zip_archive));

		mz_bool status = mz_zip_reader_init_mem(&archive, zip_data, zip_size, 0);
		if (!status)
		{
			std::cerr << "Failed to " << std::endl;
		}

		int file_count = mz_zip_reader_get_num_files(&archive);
		for (int i = 0; i < file_count; ++i)
		{
			mz_zip_archive_file_stat file_stat;
			if (!mz_zip_reader_file_stat(&archive, i, &file_stat))
			{
				std::cerr << "Failed to get file stat for index " << i << std::endl;
				continue;
			}

			std::cout << "Extracting " << file_stat.m_filename << std::endl;

			// Extrahiere die Datei in den Speicher
			size_t uncomp_size;
			void *p = mz_zip_reader_extract_file_to_heap(&archive, file_stat.m_filename, &uncomp_size, 0);
			if (!p)
			{
				std::cerr << "Failed to extract file " << file_stat.m_filename << std::endl;
				continue;
			}

			ZipFile file;
			file.Buffer = p;
			file.BufferSize = uncomp_size;
			file.Name = file_stat.m_filename;
			file.Path = file_stat.m_filename;
			result.push_back(file);
		}

		mz_zip_reader_end(&archive);

		return result;
	}

	bool ZipArchive::Store(const std::vector<ZipFile> &inZipfiles, const std::string &inFilepath)
	{
		mz_zip_archive zip_archive;
		memset(&zip_archive, 0, sizeof(mz_zip_archive));
		mz_bool status = mz_zip_writer_init_file(&zip_archive, inFilepath.c_str(), 0);
		if (!status)
		{
			std::cerr << "Failed to init zip writer." << std::endl;
			return false;
		}

		for (const Core::ZipFile &zip_file : inZipfiles)
		{
			if (!mz_zip_writer_add_mem(&zip_archive, zip_file.Name.c_str(), zip_file.Buffer, zip_file.BufferSize, MZ_BEST_COMPRESSION))
			{
				std::cerr << "Failed to add file " << zip_file.Path << " to archive " << inFilepath << std::endl;

				if (!mz_zip_writer_finalize_archive(&zip_archive))
				{
					std::cerr << "Failed to finalize zip archive." << std::endl;
				}
				mz_zip_writer_end(&zip_archive);

				return false;
			}
		}

		if (!mz_zip_writer_finalize_archive(&zip_archive))
		{
			std::cerr << "Failed to finalize zip archive." << std::endl;
		}
		mz_zip_writer_end(&zip_archive);

		return true;
	}
}
