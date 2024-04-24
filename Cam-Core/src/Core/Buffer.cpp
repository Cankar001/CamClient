#include "Buffer.h"

#include <stdlib.h>
#include <string.h>

namespace Core
{
	Buffer::Buffer()
		: Ptr(nullptr), Size(0)
	{
	}

	Buffer::~Buffer()
	{
		Free();
	}
	
	bool Buffer::Alloc(uint32 bytes)
	{
		if (!Ptr || bytes > Size)
		{
			Free();

			Ptr = (Byte*)malloc(bytes);
			if (Ptr)
			{
				Size = bytes;
				memset(Ptr, 0, Size);
				return true;
			}
		}

		return false;
	}
	
	void Buffer::Free()
	{
		if (Ptr)
		{
			memset(Ptr, 0, Size);
			free(Ptr);
			Ptr = nullptr;
		}

		Size = 0;
	}
}

