#pragma once

#include "Hud.hpp"
#include <list>

#include "Utils/SDK/Math.hpp"

class StrafeHud : public Hud {
private:
	struct {
		Vector wishDir;
		std::vector<float> accelValues;
		float maxAccel = 0;
		float minAccel = 0;
		std::list<float> precisionLog;
		float avgPrecision = 0;
	} data[2];

public:
	StrafeHud()
		: Hud(HudType_InGame, true) {}
	bool GetCurrentSize(int &w, int &h) { return false; }
	void Paint(int slot) override;
	void SetData(int slot, void *player, CUserCmd *cmd, bool serverside);
};

extern StrafeHud strafeHud;
