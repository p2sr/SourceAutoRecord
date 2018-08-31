#include "Timer.hpp"

#include "TimerAverage.hpp"
#include "TimerCheckPoints.hpp"

#include "Utils.hpp"

Timer::Timer()
    : isRunning(false)
    , isPaused(true)
    , baseTick(0)
    , totalTicks(0)
{
    this->avg = new TimerAverage();
    this->cps = new TimerCheckPoints();

    this->hasLoaded = this->avg && this->cps;
}
Timer::~Timer()
{
    SAFE_DELETE(this->avg);
    SAFE_DELETE(this->cps);
}
void Timer::Start(int engineTick)
{
    this->isRunning = true;
    this->isPaused = false;
    this->baseTick = engineTick;
    this->totalTicks = 0;
}
void Timer::Rebase(int engineTick)
{
    if (!this->isRunning)
        return;
    this->baseTick = engineTick;
    this->isPaused = false;
}
int Timer::GetTick(int engineTick)
{
    if (!this->isRunning)
        return this->totalTicks;
    int tick = engineTick - this->baseTick;
    return (tick >= 0) ? tick + this->totalTicks : this->totalTicks;
}
void Timer::Save(int engineTick)
{
    this->isPaused = true;
    this->totalTicks = this->GetTick(engineTick);
}
void Timer::Stop(int engineTick)
{
    this->Save(engineTick);
    this->isRunning = false;
}

Timer* timer;
