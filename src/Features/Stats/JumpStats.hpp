#pragma once
#include "StatsResultType.hpp"
#include "Utils/SDK.hpp"

class JumpStats {
public:
	int total;
	float distance;
	float distancePeak;
	StatsResultType type;
	bool isTracing;
	Vector source;

public:
	void StartTrace(Vector source);
	void EndTrace(Vector destination, bool xyOnly);
	void Reset();
};
