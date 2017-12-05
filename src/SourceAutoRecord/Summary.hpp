#pragma once

namespace Summary
{
	struct SummaryItem {
		int Ticks;
		float Time;
		char* Map;
	};

	bool HasStarted;

	std::vector<SummaryItem> Items;

	int TotalTicks;
	float TotalTime;

	void Start() {
		HasStarted = true;
	}
	void Reset() {
		Items.clear();
		TotalTicks = 0;
		TotalTime = 0;
		HasStarted = false;
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