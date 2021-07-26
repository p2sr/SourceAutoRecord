#pragma once

#include "Command.hpp"
#include "Hud.hpp"
#include "Variable.hpp"

class Crosshair : public Hud {
private:
	int crosshairTextureID;
	int quickhudTextureID[4];

public:
	bool isCustomCrosshairReady;
	bool isCustomQuickHudReady;
	std::vector<std::string> images;

public:
	Crosshair();
	bool ShouldDraw() override;
	void Paint(int slot) override;
	bool GetCurrentSize(int &xSize, int &ySize) override;

	bool IsSurfacePortalable();
	int GetPortalUpgradeState();
	void GetPortalsStates(int &portalUpgradeState, bool &blue, bool &orange);
	std::vector<IHandleEntity *> GetPortalsShotByPlayer();

	int SetCrosshairTexture(const std::string filename);
	bool SetQuickHudTexture(const std::string filename);

	void UpdateImages();
};

extern Crosshair crosshair;

extern Variable sar_crosshair_mode;
extern Variable sar_quickhud_mode;
extern Variable sar_crosshair_P1;

extern Variable cl_crosshair_t;
extern Variable cl_crosshairgap;
extern Variable cl_crosshaircolor_r;
extern Variable cl_crosshaircolor_g;
extern Variable cl_crosshaircolor_b;
extern Variable cl_crosshairsize;
extern Variable cl_crosshairthickness;
extern Variable cl_crosshairalpha;
extern Variable cl_crosshairdot;

extern Variable cl_quickhud_x;
extern Variable cl_quickhud_y;
extern Variable cl_quickhud_size;
extern Variable cl_quickhudleftcolor_r;
extern Variable cl_quickhudleftcolor_g;
extern Variable cl_quickhudleftcolor_b;
extern Variable cl_quickhudrightcolor_r;
extern Variable cl_quickhudrightcolor_g;
extern Variable cl_quickhudrightcolor_b;
extern Variable cl_quickhud_alpha;

extern Command sar_crosshair_set_texture;
extern Command sar_quickhud_set_texture;
