#pragma once

#include "Feature.hpp"

#include <chrono>

class TimescaleDetect : public Feature {
public:
	TimescaleDetect();
	void Cancel();
	void Spawn();

	int spawnTick;
	int startTick;
	std::chrono::time_point<std::chrono::system_clock> startTickTime;
};

extern TimescaleDetect *timescaleDetect;
