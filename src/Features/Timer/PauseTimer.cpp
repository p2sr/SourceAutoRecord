#include "PauseTimer.hpp"

PauseTimer* pauseTimer;

PauseTimer::PauseTimer()
    : isActive(false)
    , ticks(0)
{
    this->hasLoaded = true;
}
void PauseTimer::Start()
{
    this->ticks = 0;
    this->isActive = true;
}
void PauseTimer::Increment()
{
    ++this->ticks;
}
void PauseTimer::Stop()
{
    this->isActive = false;
}
bool PauseTimer::IsActive()
{
    return this->isActive;
}
int PauseTimer::GetTotal()
{
    return this->ticks;
}
