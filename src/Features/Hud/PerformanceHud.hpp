#pragma once
#include "Hud.hpp"
#include "Modules/Scheme.hpp"

class PerformanceHud : public Hud {
public:
	PerformanceHud()
		: Hud(HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen, true) {};

	bool ShouldDraw() override;

	void Paint(int slot) override;

	bool GetCurrentSize(int &xSize, int &ySize) override {
		return false;
	}

	void OnFrame(float frametime);

	unsigned accum_ticks = 0;
	std::vector<float> frametimes_offTick;
	std::vector<float> frametimes_onTick;
};

extern PerformanceHud *performanceHud;
