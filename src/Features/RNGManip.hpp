#pragma once

#include "Utils/SDK.hpp"

namespace RngManip {
	void saveData(const char *filename);
	void loadData(const char *filename);

	void viewPunch(QAngle *offset);
}
