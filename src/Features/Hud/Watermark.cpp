#include "Version.hpp"

#if defined(SAR_DEV_BUILD) && !defined(NO_DEV_WATERMARK)

#	include "Hud.hpp"
#	include "Modules/Engine.hpp"
#	include "Modules/Surface.hpp"

#	define WATERMARK_MSG "Development SAR build. Do not use."

class WatermarkHud : public Hud {
public:
	WatermarkHud()
		: Hud(HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen, false) {
	}

	bool ShouldDraw() override {
		return true;
	}

	bool GetCurrentSize(int &w, int &h) override {
		return false;
	}

	void Paint(int slot) override {
		int screenWidth, screenHeight;
		engine->GetScreenSize(nullptr, screenWidth, screenHeight);

		Surface::HFont font = 6;

		int height = surface->GetFontHeight(font);
		int width = surface->GetFontLength(font, "%s", WATERMARK_MSG);

		surface->DrawTxt(font, (screenWidth - width) / 2, screenHeight - 150, Color{255, 255, 255, 100}, "%s", WATERMARK_MSG);
	}
};

WatermarkHud watermark;

#endif
