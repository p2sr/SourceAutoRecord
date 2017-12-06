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
	float TotalTime;

	void Start() {
		Items.clear();
		TotalTicks = 0;
		TotalTime = 0;
		IsRunning = true;
	}
	void Add(int ticks, float time, char* map) {
		Items.push_back(SummaryItem {
			ticks,
			time,
			map
		});
		TotalTicks += ticks;
		TotalTime += time;
	}
}