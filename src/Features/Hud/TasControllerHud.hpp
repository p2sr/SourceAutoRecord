#pragma once
#include "Command.hpp"
#include "Hud.hpp"
#include "Variable.hpp"

#include <climits>


class TasControllerHud : public Hud {
private:
	Vector movement;
	// Storing past angles data to get the difference
	QAngle angles[2];
	bool queryNewAngles = true;
public:
	TasControllerHud();
	bool ShouldDraw() override;
	void Paint(int slot) override;
	bool GetCurrentSize(int &xSize, int &ySize) override;
	void AddData(Vector movement);
};

extern TasControllerHud tasControllerHud;

extern Variable sar_tas_controller_hud;
extern Variable sar_tas_controller_hud_offset_x;
extern Variable sar_tas_controller_hud_offset_y;
extern Variable sar_tas_controller_hud_bg;
