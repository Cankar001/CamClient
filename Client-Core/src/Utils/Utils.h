#pragma once

#include <string>

#include "Core/FileSystem.h"

namespace Core::utils
{
	// Returns the number of elements in a fixed sized array.
	template <typename T, size_t n>
	constexpr size_t Count(T const (&x)[n])
	{
		return n;
	}

	// Returns the maximum of x or y for a value type.
	template <typename type_t>
	constexpr type_t Max(type_t x, type_t y)
	{
		return (x < y) ? y : x;
	}

	// Returns the mininum of x or y for a value type.
	template <typename type_t>
	constexpr type_t Min(type_t x, type_t y)
	{
		return (y < x) ? y : x;
	}

	static bool HasMacroInText(const std::string &str, const std::string &macro)
	{
		size_t pos = str.find("#define " + macro);
		if (pos == std::string::npos)
		{
			return false;
		}

		return true;
	}

	static std::string GetMacroFromText(const std::string &str, const std::string &macro)
	{
		std::string macro_search_text = "#define " + macro;
		size_t pos = str.find(macro_search_text);
		if (pos == std::string::npos)
		{
			return "";
		}

		std::string result = str;
		result = result.substr(pos + macro_search_text.size() + 1);

		pos = result.find("\n");
		if (pos == std::string::npos)
		{
			return result;
		}

		result = result.substr(0, pos);
		return result;
	}

	static uint32 GetLocalVersion(FileSystem *filesystem, const std::string &rootPath)
	{
		std::string local_path = rootPath + "/src/CamVersion.h";
		if (!filesystem->Open(local_path, "r"))
		{
			return 0;
		}

		std::string version_content = "";
		if (!filesystem->ReadTextFile(&version_content))
		{
			return 0;
		}

		std::string local_version_str = utils::GetMacroFromText(version_content, "CAM_VERSION");
		filesystem->Close();
		return std::stoi(local_version_str);
	}
}

