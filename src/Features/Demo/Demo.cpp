#include "Demo.hpp"

#include <cstdint>

#include "SAR.hpp"

int32_t Demo::LastTick()
{
    return (this->messageTicks.size() > 0)
        ? this->messageTicks.back()
        : this->playbackTicks;
}
float Demo::IntervalPerTick()
{
    return (this->playbackTicks != 0)
        ? this->playbackTime / this->playbackTicks
        : 1 / sar.game->Tickrate();
}
float Demo::Tickrate()
{
    return (this->playbackTime != 0)
        ? this->playbackTicks / this->playbackTime
        : sar.game->Tickrate();
}
