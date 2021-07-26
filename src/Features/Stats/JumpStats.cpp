#include "JumpStats.hpp"

#include "StatsResultType.hpp"
#include "Utils/SDK.hpp"

void JumpStats::StartTrace(Vector source) {
	this->source = source;
	this->isTracing = true;
}
void JumpStats::EndTrace(Vector destination, bool xyOnly) {
	auto x = destination.x - this->source.x;
	auto y = destination.y - this->source.y;

	if (xyOnly) {
		this->distance = std::sqrt(x * x + y * y);
		this->type = StatsResultType::VEC2;
	} else {
		auto z = destination.z - source.z;
		this->distance = std::sqrt(x * x + y * y + z * z);
		this->type = StatsResultType::VEC3;
	}

	if (this->distance > this->distancePeak)
		distancePeak = distance;

	this->isTracing = false;
}
void JumpStats::Reset() {
	this->total = 0;
	this->distance = 0;
	this->distancePeak = 0;
	this->type = StatsResultType::UNKNOWN;
	this->isTracing = false;
}
