#pragma once

namespace Timer {

bool IsRunning;
bool IsPaused;

int BaseTick;
int TotalTicks;

void Start(int engineTick)
{
    IsRunning = true;
    IsPaused = false;
    BaseTick = engineTick;
    TotalTicks = 0;
}
void Rebase(int engineTick)
{
    if (!IsRunning)
        return;
    BaseTick = engineTick;
    IsPaused = false;
}
int GetTick(int engineTick)
{
    if (!IsRunning)
        return TotalTicks;
    int tick = engineTick - BaseTick;
    return (tick >= 0) ? tick + TotalTicks : TotalTicks;
}
void Save(int engineTick)
{
    IsPaused = true;
    TotalTicks = GetTick(engineTick);
}
void Stop(int engineTick)
{
    Save(engineTick);
    IsRunning = false;
}
}