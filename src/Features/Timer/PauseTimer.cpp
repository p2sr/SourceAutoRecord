#include "PauseTimer.hpp"

#include "Features/Hud/Hud.hpp"
#include "Features/Session.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Event.hpp"

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

ON_EVENT(TICK) {
    if (!server->IsRestoring() && engine->GetMaxClients() == 1) {
        if (!event.simulating && !pauseTimer->IsActive()) {
            pauseTimer->Start();
        } else if (event.simulating && pauseTimer->IsActive()) {
            pauseTimer->Stop();
            console->DevMsg("Paused for %d non-simulated ticks.\n", pauseTimer->GetTotal());
        }
    }

    if (pauseTimer->IsActive() && session->isRunning) {
        pauseTimer->Increment();
    }
}
