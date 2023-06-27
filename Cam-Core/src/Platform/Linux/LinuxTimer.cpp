#include "Core/Timer.h"

#ifdef CAM_PLATFORM_LINUX

namespace Core
{
	int64 QueryMS()
	{
		return 0;
	}

	void SleepMS(uint32 ms)
	{

	}
}

#endif // CAM_PLATFORM_LINUX

