#include "LPHud.hpp"

#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Variable.hpp"

#include <algorithm>

LPHud lpHud;

Variable sar_lphud("sar_lphud", "0", "Enables or disables the portals display on screen.\n");
Variable sar_lphud_x("sar_lphud_x", "-10", "x pos of lp counter.\n", 0);
Variable sar_lphud_y("sar_lphud_y", "-10", "y pos of lp counter.\n", 0);
Variable sar_lphud_font("sar_lphud_font", "92", 0, "Change font of portal counter.\n");

LPHud::LPHud()
	: Hud(HudType_InGame, true) {
}
bool LPHud::ShouldDraw() {
	Update();

	bool shouldDraw = sar_lphud.GetBool() && Hud::ShouldDraw();
	return shouldDraw;
}

void LPHud::Update() {
	if (!session->isRunning || engine->GetCurrentMapName().size() == 0)
		return;

	if (sar_lphud.GetBool() && !enabled) {
		oldInGamePortalCounter = -1;
		portalsCountFull = 0;
		countHistory.clear();
		enabled = true;
	} else if (!sar_lphud.GetBool() && enabled) {
		enabled = false;
	}

	if (!enabled)
		return;

	void *player = client->GetPlayer(1);
	void *player2 = client->GetPlayer(2);

	if (player && (!engine->IsCoop() || player2)) {
		int portals1 = *(int *)((uintptr_t)player + Offsets::C_m_StatsThisLevel + 4);
		int portals2 = player2 ? *(int *)((uintptr_t)player2 + Offsets::C_m_StatsThisLevel + 4) : 0;
		int iNumPortalsPlaced = portals1 + portals2;

		if (oldInGamePortalCounter != iNumPortalsPlaced) {
			if (oldInGamePortalCounter < iNumPortalsPlaced) {
				if (oldInGamePortalCounter != -1) {
					portalsCountFull += iNumPortalsPlaced - oldInGamePortalCounter;
					countHistory.push_back({engine->GetTick(), portalsCountFull});
				}
			}
			oldInGamePortalCounter = iNumPortalsPlaced;
		}
	}

	if (session->signonState >= SIGNONSTATE_NEW) {
		int currentTime = engine->GetTick();
		if (oldUpdateTick > currentTime && session->signonState != SIGNONSTATE_FULL) {
			//detect save loading or map reset/change
			if (oldLevelName.size() == 0 || oldLevelName != engine->GetCurrentMapName()) {
				//clear history of portal count on new map
				oldLevelName = engine->GetCurrentMapName();
				countHistory.clear();
				countHistory.push_back({engine->GetTick(), portalsCountFull});
			} else {
				//revert to correct history record and clear records above given time
				int closestTime = INT_MAX;
				portalsCountFull = 0;
				for (auto info : countHistory) {
					int dif = currentTime - info.tick;
					if (dif >= 0 && dif < closestTime) {
						closestTime = dif;
						portalsCountFull = info.count;
					}
				}
				countHistory.erase(
					std::remove_if(countHistory.begin(), countHistory.end(), [currentTime](auto &x) { return x.tick > currentTime; }),
					countHistory.end());
			}
		}
		oldUpdateTick = currentTime;
	}
}

void LPHud::Paint(int slot) {
	auto font = scheme->GetDefaultFont() + sar_lphud_font.GetInt();

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

	int digitCount = fmax(ceil(log10(portalsCountFull + 1)), 1);
	surface->DrawTxt(font, cX + bgWidth - digitWidth * digitCount - paddingSide, cY + paddingTop, Color(255, 255, 255, 255), "%d", portalsCountFull);
}

bool LPHud::GetCurrentSize(int &xSize, int &ySize) {
	auto font = scheme->GetDefaultFont() + sar_lphud_font.GetInt();

	int digitWidth = surface->GetFontLength(font, "3");
	int charHeight = surface->GetFontHeight(font);
	xSize = surface->GetFontLength(font, "Portals:") * 2;
	ySize = (int)(charHeight * 1.5);

	return true;
}

void LPHud::Set(int count) {
	portalsCountFull = count;
	countHistory.push_back({engine->GetTick(), count});
}

CON_COMMAND(sar_lphud_set, "sar_lphud_set <number> - sets lp counter to given number\n") {
	IGNORE_DEMO_PLAYER();

	if (args.ArgC() != 2) {
		return console->Print(sar_lphud_set.ThisPtr()->m_pszHelpString);
	}

	lpHud.Set(static_cast<int>(std::atoi(args[1])));
}

CON_COMMAND_HUD_SETPOS(sar_lphud, "least portals HUD")
