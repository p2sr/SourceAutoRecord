#pragma once
#include "StatsResultType.hpp"

struct Vector;

class VelocityStats {
public:
	float peak;
	StatsResultType type;

public:
	void Save(Vector velocity, bool xyOnly);
	void Reset();
};
