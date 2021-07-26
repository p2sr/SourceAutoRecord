#pragma once
#include <string>
#include <vector>

struct TimerCheckPointItem {
	int ticks;
	float time;
	std::string map;
};

class TimerCheckPoints {
public:
	std::vector<TimerCheckPointItem> items;
	int latestTick;
	float latestTime;

public:
	TimerCheckPoints();
	void Add(int ticks, float time, std::string map);
	void Reset();
};
