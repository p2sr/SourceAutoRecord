#pragma once

#include "Utils/SDK.hpp"

#define RNG_MANIP_EXT "p2rng"

namespace RngManip {
	void saveData(const char *filename);
	void loadData(const char *filename);

	void viewPunch(QAngle *offset);
	void randomSeed(int *seed);
}
