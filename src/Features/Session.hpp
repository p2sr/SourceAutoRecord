#pragma once
#include "Feature.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"

#include <chrono>
#include <string>

class Session : public Feature {
public:
	int baseTick;
	int lastSession;

	bool isRunning;
	unsigned currentFrame;
	unsigned lastFrame;
	int prevState;
	std::string previousMap;
	int oldFpsMax;

	std::chrono::time_point<std::chrono::high_resolution_clock> loadStart;
	std::chrono::time_point<std::chrono::high_resolution_clock> loadEnd;
	int signonState;

public:
	Session();
	int GetTick();
	void Rebase(const int from);
	void Started(bool menu = false);
	void Start();
	void Ended();
	virtual void Changed();
	void Changed(int state);

	void DoFastLoads();
	void ResetLoads();
};

extern Session *session;

extern Variable sar_loads_uncap;
extern Variable sar_loads_norender;
