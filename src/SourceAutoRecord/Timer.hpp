#pragma once

namespace Timer
{
	bool IsRunning;

	int FirstTick;
	int LastTick;

	void Start(int tick) {
		FirstTick = tick;
		LastTick = 0;
		IsRunning = true;
	}
	void Stop(int tick) {
		LastTick = tick;
		IsRunning = false;
	}
	int GetTick(int current = -1) {
		int tick = (current != -1) ? current - FirstTick : LastTick - FirstTick;
		return (tick >= 0) ? tick : 0;
	}
}