#pragma once
#include "Features/Feature.hpp"

class PauseTimer : public Feature {
private:
	bool isActive;
	int ticks;

public:
	PauseTimer();
	void Start();
	void Increment();
	void Stop();
	bool IsActive();
	int GetTotal();
};

extern PauseTimer *pauseTimer;
