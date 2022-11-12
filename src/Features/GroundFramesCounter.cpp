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

HUD_ELEMENT_MODE2(groundframes, "0", 0, 2, "Draws the number of ground frames since last landing. Setting it to 2 preserves the value.\n", HudType_InGame | HudType_Paused | HudType_LoadingScreen) {
	ctx->DrawElement("groundframes: %d", groundFramesCounter->counter[ctx->slot]);
}

void GroundFramesCounter::HandleMovementFrame(int slot, bool grounded) {
	if (pauseTimer->IsActive() && !engine->IsCoop() && !engine->demoplayer->IsPlaying()) return;

	if (!this->grounded[slot] && grounded) {
		this->AddToTotal(slot, this->counter[slot]);
	}

	int hudMode = sar_hud_groundframes.GetInt();

	if ((!this->grounded[slot] && grounded) || hudMode == 0) {
		if (hudMode != 2) this->counter[slot] = 0;
	} else if (grounded) {
		this->counter[slot]++;
	}

	this->grounded[slot] = grounded;
}

void GroundFramesCounter::AddToTotal(int slot, int count) {
	if (count >= MAX_GROUNDFRAMES_TRACK) return;
	this->totals[slot][count] += 1;
}

CON_COMMAND(sar_groundframes_total, "sar_groundframes_total [slot] - output a summary of groundframe counts for the given player slot.\n") {
	int slot = args.ArgC() > 1 ? atoi(args[1]) : 0;
	if (slot < 0) slot = 0;
	if (slot > 1) slot = 1;

	int total_hops = 0;
	for (int i = 0; i < MAX_GROUNDFRAMES_TRACK; ++i) {
		total_hops += groundFramesCounter->totals[slot][i];
	}

	if (total_hops == 0) total_hops = 1;

	for (int i = 0; i < MAX_GROUNDFRAMES_TRACK; ++i) {
		int hops = groundFramesCounter->totals[slot][i];
		console->Print("%d groundframes: %d hops (%.1f%%)\n", i, hops, (float)hops / (float)total_hops * 100.0f);
	}
}

CON_COMMAND(sar_groundframes_reset, "sar_groundframes_reset - reset recorded groundframe statistics.\n") {
	for (int i = 0; i < MAX_GROUNDFRAMES_TRACK; ++i) {
		groundFramesCounter->totals[0][i] = 0;
		groundFramesCounter->totals[1][i] = 0;
	}
}

HUD_ELEMENT_MODE2(grounded, "0", 0, 1, "Draws the state of player being on ground.\n", HudType_InGame | HudType_Paused | HudType_LoadingScreen) {
	ctx->DrawElement("grounded: %s", groundFramesCounter->grounded[ctx->slot] ? "yes" : "no");
}
