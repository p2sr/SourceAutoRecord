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

	void OnJump(int slot);
};

extern ScrollSpeedHud scrollSpeedHud;
