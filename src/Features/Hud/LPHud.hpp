#pragma once
#include "Command.hpp"
#include "Hud.hpp"
#include "Variable.hpp"

#include <climits>

struct LPHudCountHistoryInfo {
	int tick = 0;
	int count = 0;
};

class LPHud : public Hud {
private:
	int oldInGamePortalCounter = 0;
	int portalsCountFull = 0;
	bool enabled = false;
	std::vector<LPHudCountHistoryInfo> countHistory;
	std::string oldLevelName;
	int oldUpdateTick = INT_MAX;

public:
	LPHud();
	bool ShouldDraw() override;
	void Paint(int slot) override;
	bool GetCurrentSize(int &xSize, int &ySize) override;
	void Set(int count);
	void Update();
};

extern LPHud lpHud;

extern Variable sar_lphud;
extern Variable sar_lphud_x;
extern Variable sar_lphud_y;
extern Variable sar_lphud_font;

extern Command sar_lphud_set;
