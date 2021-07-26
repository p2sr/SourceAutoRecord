#pragma once

#include "Hud.hpp"

class PortalgunHud : public Hud {
public:
	PortalgunHud();
	bool ShouldDraw() override;
	bool GetCurrentSize(int &w, int &h) override;
	void Paint(int slot) override;
};

extern PortalgunHud portalgunHud;
