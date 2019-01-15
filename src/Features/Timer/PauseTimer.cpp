#include "PauseTimer.hpp"

#include "Variable.hpp"

Variable sar_time_pauses("sar_time_pauses", "1", "Counts non-simulated ticks when the server pauses. "
                                                 "Setting this to 0 also affects sar_timer_time_pauses and sar_speedrun_time_pauses.\n");

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
