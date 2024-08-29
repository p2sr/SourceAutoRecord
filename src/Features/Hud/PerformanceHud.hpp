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

	void AddMetric(std::vector<float> &type, float frametime);
	void OnFrame(float frametime);

	unsigned accum_ticks = 0;
	std::vector<float> times_totalframe_offTick;
	std::vector<float> times_totalframe_onTick;
	std::vector<float> times_render;
	std::vector<float> times_vgui;
	std::vector<float> times_preTick;
	std::vector<float> times_postTick;
};

extern PerformanceHud *performanceHud;
