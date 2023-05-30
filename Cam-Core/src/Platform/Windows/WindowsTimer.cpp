#include "Core/Timer.h"

#ifdef CAM_PLATFORM_WINDOWS

#include <Windows.h>

namespace Core
{
	int64 QueryMS()
	{
		return (int64)GetTickCount64();
	}

	void SleepMS(uint32 ms)
	{
		Sleep(ms);
	}
}

#endif // CAM_PLATFORM_WINDOWS

