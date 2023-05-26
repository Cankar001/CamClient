#pragma once

#include "Core.h"

namespace Core
{
	// Returns the current time in milliseconds.
	int64 QueryMS();

	// Sleeps for the given number of milliseconds.
	void SleepMS(uint32 ms);
}

