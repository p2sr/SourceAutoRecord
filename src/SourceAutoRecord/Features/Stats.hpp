#pragma once

namespace Stats
{
	uint32_t TotalJumps;
	uint32_t TotalSteps;

	void Reset()
	{
		TotalJumps = 0;
		TotalSteps = 0;
	}
}