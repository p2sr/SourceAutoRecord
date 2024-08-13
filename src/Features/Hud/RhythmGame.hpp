#pragma once
#include "Hud.hpp"
#include "Modules/Scheme.hpp"

class RhythmGameHud : public Hud {
public:
	RhythmGameHud()
		: Hud(HudType_InGame, true){};

	bool ShouldDraw() override;

	void Paint(int slot) override;

	bool GetCurrentSize(int &xSize, int &ySize) override {
		return false;
	}

	void OnJump(int slot);

	void HandleGroundframeLogic(int slot, bool grounded);

	int groundframes = 0;

	struct RhythmGamePopup {
		int x;
		int y;
		int alpha;
		int lifetime;
		int type;
		int streak;
	};

	int perfectsInARow = 0;

	std::vector<RhythmGamePopup> popups;
	Color perfectColor = Color{0, 171, 255, 0};  // 0 groundframes
	Color goodColor = Color{0, 236, 82, 0};     // 1 groundframe
	Color okColor = Color{127, 127, 127, 0};       // 2 groundframes
	Color badColor = Color{216, 0, 0, 0};      // 3-6 groundframes

private:
	int GetGroundframes();
};

extern RhythmGameHud rhythmGameHud;
