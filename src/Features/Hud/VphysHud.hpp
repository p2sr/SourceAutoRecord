#pragma once
#include "Command.hpp"
#include "Hud.hpp"
#include "Variable.hpp"

struct VphysShadowInfo {
	void *player;
	void *shadow;
	bool asleep;
	bool collisionEnabled;
	bool gravityEnabled;
	Vector position;
	QAngle rotation;
	Vector velocity;
	Vector angularVelocity;
};

class VphysHud : public Hud {
public:
	VphysHud();
	bool ShouldDraw() override;
	void Paint(int slot) override;
	bool GetCurrentSize(int &xSize, int &ySize) override;

	VphysShadowInfo GetVphysInfo(int slot, bool crouched);
};

extern VphysHud vphysHud;

extern Variable sar_vphys_hud;
extern Variable sar_vphys_hud_x;
extern Variable sar_vphys_hud_y;

extern Command sar_vphys_setgravity;
extern Command sar_vphys_setangle;
extern Command sar_vphys_setspin;
