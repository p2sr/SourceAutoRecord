#pragma once

#include "Hud.hpp"

class StrafeQualityHud : public Hud {
public:
	StrafeQualityHud();
	bool ShouldDraw() override;
	bool GetCurrentSize(int &width, int &height) override;
	void Paint(int slot) override;
	void OnUserCmd(int slot, const CUserCmd &cmd);
	void OnMovement(int slot, bool grounded);
};

extern StrafeQualityHud strafeQuality;
