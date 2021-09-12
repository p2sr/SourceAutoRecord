#pragma once
#include "Hud.hpp"
#include "Variable.hpp"

class VelocityGraph : public Hud {
public:
	VelocityGraph();
	bool ShouldDraw() override;
	void GatherData(int slot);
	Color HSVtoRGB(float H, float S, float V);
	void Paint(int slot) override;
	bool GetCurrentSize(int &xSize, int &ySize) override;
};

extern VelocityGraph velocityGraph;

extern Variable sar_velocitygraph;
extern Variable sar_velocitygraph_font_index;
