#include "VelocityStats.hpp"

#include "StatsResultType.hpp"
#include "Utils/SDK.hpp"

void VelocityStats::Save(Vector velocity, bool xyOnly) {
	auto vel = 0.f;
	if (xyOnly) {
		vel = velocity.Length2D();
		this->type = StatsResultType::VEC2;
	} else {
		vel = velocity.Length();
		this->type = StatsResultType::VEC3;
	}

	if (vel > this->peak)
		this->peak = vel;
}
void VelocityStats::Reset() {
	this->peak = 0;
	this->type = StatsResultType::UNKNOWN;
}
