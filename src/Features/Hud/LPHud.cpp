#include "LPHud.hpp"

#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Variable.hpp"
#include "Event.hpp"

#include <algorithm>

LPHud lpHud;

Variable sar_lphud("sar_lphud", "0", "Enables or disables the portals display on screen.\n");
Variable sar_lphud_x("sar_lphud_x", "-10", "x pos of lp counter.\n", 0);
Variable sar_lphud_y("sar_lphud_y", "-10", "y pos of lp counter.\n", 0);
Variable sar_lphud_font("sar_lphud_font", "92", 0, "Change font of portal counter.\n");
Variable sar_lphud_reset_on_changelevel("sar_lphud_reset_on_changelevel", "0", "Reset the lp counter on any changelevel or restart_level. Useful for ILs.\n");

struct LPHudCountHistoryInfo {
	std::string map;
	int tick;
	int count;
	bool changelevel; // Was this count update triggered by a changelevel?
};

static std::vector<LPHudCountHistoryInfo> g_count_history;
static int g_cur_history_idx = -1; // Cannot be size_t; may be -1
static int g_stats_count = 0; // The last portal count as reported by m_StatsThisLevel

// Get the portal count as reported by the current count history
static int getCurrentCount() {
	if (g_cur_history_idx == -1) return 0;
	return g_count_history[g_cur_history_idx].count;
}

// Get the current total portal count as reported by the the players'
// m_StatsThisLevel values
static int getStatsCount() {
	int total = 0;

	int slots = engine->GetMaxClients() >= 2 ? 2 : 1;
	for (int slot = 0; slot < slots; ++slot) {
		void *player = client->GetPlayer(slot + 1);
		if (!player) continue;
		total += *(int *)((uintptr_t)player + Offsets::C_m_StatsThisLevel + 4);
	}

	return total;
}

// Calculate an up-to-date total portal count using the player stats and
// the current count history
static int calcTotalPortals() {
	int delta = getStatsCount() - g_stats_count;
	if (delta > 0) {
		// We have new portals!
		return getCurrentCount() + delta;
	}
	return getCurrentCount();
}

// Add the latest portal count to the history
static void addNewCount(int count, bool changelevel = false) {
	if (g_cur_history_idx != g_count_history.size() - 1) {
		// Destroy all future history
		g_count_history.resize(g_cur_history_idx + 1);
	}

	g_count_history.push_back({
		engine->GetCurrentMapName(),
		engine->GetTick(),
		count,
		changelevel,
	});

	g_stats_count = getStatsCount();

	++g_cur_history_idx;
}

LPHud::LPHud()
	: Hud(HudType_InGame, true) {
}
bool LPHud::ShouldDraw() {
	return sar_lphud.GetBool() && Hud::ShouldDraw();
}

ON_EVENT(SESSION_START) {
	g_stats_count = getStatsCount(); // Should always be 0 I think

	if (event.transition) {
		// Awesome, it was a level transition! Push the new count, but don't
		// recalculate (we can't yet, we don't have a valid stats count in
		// the history)
		addNewCount(getCurrentCount());
	} else if (event.load) {
		// Oh god. Try to restore by going back to the last count on an
		// engine tick less than or equal to this one
		int tick = engine->GetTick();
		while (g_cur_history_idx >= 0 && (g_count_history[g_cur_history_idx].tick > tick || g_count_history[g_cur_history_idx].changelevel)) {
			--g_cur_history_idx;
		}
	} else {
		// It was a restart_level or something like that
		if (sar_lphud_reset_on_changelevel.GetBool()) {
			lpHud.Set(0);
		} else {
			// Restore the portal count from the first entry for this map, or
			// otherwise just go to the latest one
			auto map = engine->GetCurrentMapName();
			g_cur_history_idx = g_count_history.size() - 1;
			for (int i = 0; i < g_count_history.size(); ++i) {
				if (g_count_history[i].map == map) {
					g_cur_history_idx = i;
					break;
				}
			}

			if (g_cur_history_idx == -1 || g_count_history[g_cur_history_idx].map != map) {
				// Push a history for this map, like for a transition
				addNewCount(getCurrentCount(), true);
			}
		}
	}
}

ON_EVENT(POST_TICK) {
	if (!session->isRunning) return;
	int total = calcTotalPortals();
	if (getCurrentCount() != total) {
		addNewCount(total);
	}
}

void LPHud::Paint(int slot) {
	auto font = scheme->GetFontByID(sar_lphud_font.GetInt());

	int cX = PositionFromString(sar_lphud_x.GetString(), true);
	int cY = PositionFromString(sar_lphud_y.GetString(), false);

	int digitWidth = surface->GetFontLength(font, "3");
	int charHeight = surface->GetFontHeight(font);
	int bgWidth = surface->GetFontLength(font, "Portals:") * 2;
	int bgHeight = (int)(charHeight * 1.5);

	int paddingTop = (int)(charHeight * 0.22);
	int paddingSide = (int)(digitWidth * 1);

	surface->DrawRect(Color(0, 0, 0, 192), cX, cY, cX + bgWidth, cY + bgHeight);

	surface->DrawTxt(font, cX + paddingSide, cY + paddingTop, Color(255,255,255,255), "Portals:");

	int portals = getCurrentCount();

	int digitCount = fmax(ceil(log10(portals + 1)), 1);
	surface->DrawTxt(font, cX + bgWidth - digitWidth * digitCount - paddingSide, cY + paddingTop, Color(255, 255, 255, 255), "%d", portals);
}

bool LPHud::GetCurrentSize(int &xSize, int &ySize) {
	auto font = scheme->GetFontByID(sar_lphud_font.GetInt());

	int digitWidth = surface->GetFontLength(font, "3");
	int charHeight = surface->GetFontHeight(font);
	xSize = surface->GetFontLength(font, "Portals:") * 2;
	ySize = (int)(charHeight * 1.5);

	return true;
}

void LPHud::Set(int count) {
	// Just clear the history and add this
	g_count_history.clear();
	g_cur_history_idx = -1;
	addNewCount(count);
}

CON_COMMAND(sar_lphud_set, "sar_lphud_set <number> - sets lp counter to given number\n") {
	IGNORE_DEMO_PLAYER();

	if (args.ArgC() != 2) {
		return console->Print(sar_lphud_set.ThisPtr()->m_pszHelpString);
	}

	lpHud.Set(std::atoi(args[1]));
}

CON_COMMAND(sar_lphud_reset, "sar_lphud_reset - resets lp counter\n") {
	IGNORE_DEMO_PLAYER();

	if (args.ArgC() != 1) {
		return console->Print(sar_lphud_reset.ThisPtr()->m_pszHelpString);
	}

	lpHud.Set(0);
}

CON_COMMAND_HUD_SETPOS(sar_lphud, "least portals HUD")
