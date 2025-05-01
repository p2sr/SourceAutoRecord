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
