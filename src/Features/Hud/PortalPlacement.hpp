#pragma once

#include "Command.hpp"
#include "Hud.hpp"
#include "Variable.hpp"

class PortalPlacementHud : public Hud {
public:
    PortalPlacementHud();
    bool ShouldDraw() override;
	void Paint(int slot) override;
	bool GetCurrentSize(int &xSize, int &ySize) override;
};

extern PortalPlacementHud portalplacementHud;

extern Variable sar_pp_hud;
extern Variable sar_pp_hud_show_blue;
extern Variable sar_pp_hud_show_orange;
extern Variable sar_pp_hud_x;
extern Variable sar_pp_hud_y;
extern Variable sar_pp_hud_opacity;
extern Variable sar_pp_hud_font;
