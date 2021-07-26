#pragma once
#include "Hud.hpp"
#include "Variable.hpp"

class InspectionHud : public Hud {
public:
	InspectionHud();
	bool GetCurrentSize(int &xSize, int &ySize) override;
	bool ShouldDraw() override;
	void Paint(int slot) override;
};

extern InspectionHud inspectionHud;

extern Variable sar_ei_hud;
extern Variable sar_ei_hud_x;
extern Variable sar_ei_hud_y;
extern Variable sar_ei_hud_z;
extern Variable sar_ei_hud_font_color;
extern Variable sar_ei_hud_font_color2;
extern Variable sar_ei_hud_font_index;
