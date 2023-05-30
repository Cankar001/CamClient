#pragma once

#include "Core.h"

namespace Core
{
	uint32 Crc32(void const *src, uint32 bytes);
	uint32 Mix32(uint32 x);
	uint64 Mix64(uint64 x);
	uint64 Raw64(void const *src, uint32 bytes);
	uint32 Text(char const *src);
}

