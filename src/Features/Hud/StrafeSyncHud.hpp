#pragma once
#include "Hud.hpp"
#include "Variable.hpp"

class StrafeSyncHud : public Hud {
public:
	StrafeSyncHud();
	bool ShouldDraw() override;
	void Paint(int slot) override;
	bool GetCurrentSize(int &xSize, int &ySize) override;
};

extern StrafeSyncHud strafeSyncHud;

extern Variable sar_hud_strafesync_offset_x;
extern Variable sar_hud_strafesync_offset_y;
extern Variable sar_hud_strafesync_split_offset_y;
extern Variable sar_hud_strafesync_color;
extern Variable sar_hud_strafesync_font_index;
