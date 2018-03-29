#pragma once

namespace Summary
{
	struct SummaryItem {
		int Ticks;
		float Time;
		char* Map;
	};

	bool IsRunning;

	std::vector<SummaryItem> Items;

	int TotalTicks;

	void Start()
	{
		Items.clear();
		TotalTicks = 0;
		IsRunning = true;
	}
	void Add(int ticks, float time, char* map)
	{
		Items.push_back(SummaryItem
		{
			ticks,
			time,
			map
		});
		TotalTicks += ticks;
	}
}