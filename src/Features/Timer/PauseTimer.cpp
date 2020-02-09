#include "PauseTimer.hpp"

#include "Features/Hud/Hud.hpp"

#include "Modules/Engine.hpp"

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

// HUD

HUD_ELEMENT(pause_timer, "0", "Draws current value of pause timer.\n", HudType_InGame | HudType_Paused)
{
    auto tick = pauseTimer->GetTotal();
    auto time = engine->ToTime(tick);
    ctx->DrawElement("pause: %i (%.3f)", tick, time);
}
