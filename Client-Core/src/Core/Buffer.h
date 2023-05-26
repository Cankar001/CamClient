#pragma once

#include "Core.h"

namespace Core
{
	class Buffer
	{
	public:

		Byte *Ptr = nullptr;
		uint32 Size = 0;

		Buffer();
		~Buffer();

		bool Alloc(uint32 bytes);
		void Free();
	};
}