#pragma once
#include "Utils/SDK.hpp"

struct ReplayFrame {
	QAngle viewangles;
	float forwardmove;
	float sidemove;
	float upmove;
	int buttons;
	unsigned char impulse;
	short mousedx;
	short mousedy;
};
