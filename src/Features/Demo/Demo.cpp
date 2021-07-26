#include "Demo.hpp"

#include "SAR.hpp"

#include <cstdint>

int32_t Demo::LastTick() {
	return (!this->messageTicks.empty())
		? this->messageTicks.back()
		: this->playbackTicks;
}
float Demo::IntervalPerTick() {
	return (this->playbackTicks != 0)
		? this->playbackTime / this->playbackTicks
		: 1 / sar.game->Tickrate();
}
float Demo::Tickrate() {
	return (this->playbackTime != 0)
		? this->playbackTicks / this->playbackTime
		: sar.game->Tickrate();
}
