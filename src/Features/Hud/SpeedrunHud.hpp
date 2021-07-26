#pragma once
#include "Hud.hpp"
#include "Variable.hpp"

class SpeedrunHud : public Hud {
public:
	SpeedrunHud();
	bool ShouldDraw() override;
	void Paint(int slot) override;
	bool GetCurrentSize(int &xSize, int &ySize) override;
};

extern SpeedrunHud speedrunHud;

extern Variable sar_sr_hud;
extern Variable sar_sr_hud_x;
extern Variable sar_sr_hud_y;
extern Variable sar_sr_hud_font_color;
extern Variable sar_sr_hud_font_index;
