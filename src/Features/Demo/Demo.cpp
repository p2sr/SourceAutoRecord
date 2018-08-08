#include "Demo.hpp"

int32_t Demo::LastTick()
{
    return (this->messageTicks.size() > 0) ? this->messageTicks.back() : this->playbackTicks;
}
float Demo::IntervalPerTick()
{
    return this->playbackTime / this->playbackTicks;
}
float Demo::Tickrate()
{
    return this->playbackTicks / this->playbackTime;
}
