#pragma once

namespace Stats
{
	uint32_t TotalJumps;
	uint32_t TotalUses;

	void Reset()
	{
		TotalJumps = 0;
		TotalUses = 0;
	}
}