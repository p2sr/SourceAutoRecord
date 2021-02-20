#include "GroundFramesCounter.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"

#include "Hud/Hud.hpp"

#include "Features/Timer/PauseTimer.hpp"

GroundFramesCounter* groundFramesCounter;

GroundFramesCounter::GroundFramesCounter()
{
    this->hasLoaded = true;
}

void GroundFramesCounter::HandleJump()
{
    if (!grounded) {
        counter = 0;
    }
}

void GroundFramesCounter::HandleMovementFrame(bool newGrounded)
{
    if (pauseTimer->IsActive()) return;

    if (newGrounded) {
        counter++;
    }

    if (!grounded && newGrounded) {
        counter = 0;
    }

    grounded = newGrounded;
}


HUD_ELEMENT_MODE2(groundframes, "0", 0, 1, "Draws the number of ground frames since last landing.\n",
    HudType_InGame | HudType_Paused | HudType_LoadingScreen)
{
    ctx->DrawElement("groundframes: %d", groundFramesCounter->counter);
}
