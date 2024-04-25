#include "Core.h"

#include "Core/Log.h"

namespace Core
{
	void Init()
	{
	}

	void Shutdown()
	{
		if (Logger::Get())
		{
		//	delete Logger::Get();
		}
	}
}