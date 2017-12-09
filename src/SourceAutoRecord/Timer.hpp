#pragma once

namespace Timer
{
	bool IsRunning;

	int FirstTick;
	float FirstTime;

	int LastTick;
	float LastTime;

	void Start(int tick, float time) {
		FirstTick = tick;
		FirstTime = time;
		LastTick = 0;
		LastTime = 0;
		IsRunning = true;
	}
	void Stop(int tick, float time) {
		LastTick = tick;
		LastTime = time;
		IsRunning = false;
	}
	int GetTick() {
		int tick = LastTick - FirstTick;
		return (tick >= 0) ? tick : 0;
	}
	float GetTime() {
		int time = LastTime - FirstTime;
		return (time >= 0) ? time : 0;
	}
}