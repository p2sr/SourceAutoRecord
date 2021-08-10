#pragma once

#include "Hud.hpp"

#include <vector>

class AimPointHud : public Hud {
public:
	AimPointHud()
		: Hud(HudType_InGame, true)
	{ }

	bool ShouldDraw() override;
	bool GetCurrentSize(int &xSize, int &ySize) override;
	void Paint(int slot) override;
};

extern AimPointHud aimPointHud;
