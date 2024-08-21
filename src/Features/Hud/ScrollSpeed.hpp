#include "Hud.hpp"

class ScrollSpeedHud : public Hud {
public:
	ScrollSpeedHud()
		: Hud(HudType_InGame, true) {}

	bool ShouldDraw() override;

	void Paint(int slot) override;

	bool GetCurrentSize(int &xSize, int &ySize) override {
		return false;
	}

	static void OnJump(int slot, bool grounded);
};

extern ScrollSpeedHud scrollSpeedHud;

extern Variable sar_scrollspeed;
extern Variable sar_scrollspeed_x;
extern Variable sar_scrollspeed_y;
extern Variable sar_scrollspeed_bar_x;
extern Variable sar_scrollspeed_bar_y;