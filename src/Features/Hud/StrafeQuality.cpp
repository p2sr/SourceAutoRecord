#include "StrafeQuality.hpp"

#include "Features/Session.hpp"
#include "Features/Timer/PauseTimer.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Surface.hpp"

#include <cstring>
#include <vector>

#define TICKS_AVG 40

enum class StrafeDir {
	NONE,
	LEFT,
	RIGHT,
};

struct TickInfo {
	bool grounded;
	bool landed;
	bool jumped;
	StrafeDir strafe;
	int mouseDelta;
};

static std::vector<TickInfo> g_ticks[2];
static CUserCmd g_lastUserCmd[2];
static int g_lastMouseDeltas[2];

StrafeQualityHud strafeQuality;

static Variable sar_strafe_quality("sar_strafe_quality", "0", "Enables or disables the strafe quality HUD.\n");
static Variable sar_strafe_quality_ticks("sar_strafe_quality_ticks", "40", 10, "The number of ticks to average over for the strafe quality HUD.\n");
static Variable sar_strafe_quality_width("sar_strafe_quality_width", "300", 10, "The width of the strafe quality HUD.\n");
static Variable sar_strafe_quality_height("sar_strafe_quality_height", "50", 10, "The height of the strafe quality HUD.\n");

StrafeQualityHud::StrafeQualityHud()
	: Hud(HudType_InGame | HudType_Paused, true) {
}

bool StrafeQualityHud::ShouldDraw() {
	return Hud::ShouldDraw() && sar_strafe_quality.GetBool();
}

bool StrafeQualityHud::GetCurrentSize(int &width, int &height) {
	return false;
}

static inline void getSyncForTick(const TickInfo &tick, int *totalStrafe, int *syncedStrafe) {
	*totalStrafe = *syncedStrafe = 0;
	if (tick.strafe != StrafeDir::NONE && !tick.grounded) {
		*totalStrafe = tick.mouseDelta < 0 ? -tick.mouseDelta : tick.mouseDelta;
		if (tick.mouseDelta < 0 && tick.strafe == StrafeDir::LEFT) {
			*syncedStrafe = -tick.mouseDelta;
		} else if (tick.mouseDelta > 0 && tick.strafe == StrafeDir::RIGHT) {
			*syncedStrafe = tick.mouseDelta;
		}
	}
}

static inline void purgeOldTicks(int slot) {
	while (g_ticks[slot].size() >= sar_strafe_quality_width.GetInt() + sar_strafe_quality_ticks.GetInt()) {
		g_ticks[slot].erase(g_ticks[slot].begin());
	}
}

void StrafeQualityHud::Paint(int slot) {
	purgeOldTicks(slot);  // Just to be safe

	auto &ticks = g_ticks[slot];

	int ticksAvg = sar_strafe_quality_ticks.GetInt();

	int screenWidth, screenHeight;
	engine->GetScreenSize(nullptr, screenWidth, screenHeight);

	int width = sar_strafe_quality_width.GetInt();
	int height = sar_strafe_quality_height.GetInt();

	int x = screenWidth / 2 - width / 2;
	int y = screenHeight - height - 16;

	surface->DrawRect({75, 75, 75, 255}, x - 1, y - 1, x + width + 1, y + height + 1);
	surface->DrawRect({0, 0, 0, 255}, x, y, x + width, y + height);

	int curX = x;

	int totalMouse = 0;
	int syncedMouse = 0;

	for (int i = 0; i < ticks.size(); ++i) {
		auto &tick = ticks[i];

		int tickTotal, tickSynced;
		getSyncForTick(tick, &tickTotal, &tickSynced);
		totalMouse += tickTotal;
		syncedMouse += tickSynced;

		if (i - ticksAvg >= 0) {
			int oldTotal, oldSynced;
			getSyncForTick(ticks[i - ticksAvg], &oldTotal, &oldSynced);
			totalMouse -= oldTotal;
			syncedMouse -= oldSynced;
		}

		if (i >= ticksAvg - 1) {
			Color bg =
				tick.jumped ? Color{0, 80, 120, 255} : tick.grounded && !tick.landed ? Color{120, 40, 40, 255}
																																																																									: Color{50, 120, 50, 255};

			surface->DrawRect(bg, curX, y, curX + 1, y + height);

			float sync = totalMouse == 0 ? 0.5 : (float)syncedMouse / (float)totalMouse;
			int qualHeight = (sync - 0.5f) * (height - 3);

			surface->DrawRect({200, 200, 200, 255}, curX, y + height / 2 - qualHeight - 1, curX + 1, y + height / 2 - qualHeight + 2);

			++curX;
		}
	}

	surface->DrawRect({180, 180, 180, 255}, x, y + height / 2, x + width, y + height / 2 + 1);
}
void StrafeQualityHud::OnUserCmd(int slot, const CUserCmd &cmd) {
	g_lastUserCmd[slot] = cmd;
	g_lastMouseDeltas[slot] += cmd.mousedx;
}
void StrafeQualityHud::OnMovement(int slot, bool grounded) {
	if (pauseTimer->IsActive() && !engine->IsCoop() && !engine->demoplayer->IsPlaying()) return;

	CUserCmd &cmd = g_lastUserCmd[slot];

	StrafeDir strafe =
		(cmd.buttons & IN_MOVELEFT) && !(cmd.buttons & IN_MOVERIGHT) ? StrafeDir::LEFT : (cmd.buttons & IN_MOVERIGHT) && !(cmd.buttons & IN_MOVELEFT) ? StrafeDir::RIGHT
																																																																																																																																																: StrafeDir::NONE;

	int mouseDelta = g_lastMouseDeltas[slot];
	g_lastMouseDeltas[slot] = 0;

	bool landed = false;
	bool jumped = false;

	if (g_ticks[slot].size() > 0) {
		TickInfo &prev = g_ticks[slot].back();
		landed = grounded && !prev.grounded;
		jumped = prev.grounded && !grounded;
	}

	purgeOldTicks(slot);

	g_ticks[slot].push_back({
		grounded,
		landed,
		jumped,
		strafe,
		mouseDelta,
	});
}
