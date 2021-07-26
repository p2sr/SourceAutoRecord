#include "GroundFramesCounter.hpp"

#include "Features/Timer/PauseTimer.hpp"
#include "Hud/Hud.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

GroundFramesCounter *groundFramesCounter;

GroundFramesCounter::GroundFramesCounter() {
	this->hasLoaded = true;
}

void GroundFramesCounter::HandleMovementFrame(int slot, bool grounded) {
	if (pauseTimer->IsActive() && !engine->IsCoop() && !engine->demoplayer->IsPlaying()) return;

	if (!this->grounded[slot] && grounded) {
		this->counter[slot] = 0;
	} else if (grounded) {
		this->counter[slot]++;
	}

	this->grounded[slot] = grounded;
}


HUD_ELEMENT_MODE2(groundframes, "0", 0, 1, "Draws the number of ground frames since last landing.\n", HudType_InGame | HudType_Paused | HudType_LoadingScreen) {
	ctx->DrawElement("groundframes: %d", groundFramesCounter->counter[ctx->slot]);
}
