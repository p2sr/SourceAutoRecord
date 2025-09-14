#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Modules/Engine.hpp"
#include "Version.hpp"

#if defined(SAR_DEV_BUILD) && !defined(NO_DEV_WATERMARK)

#	include "Hud.hpp"
#	include "Modules/Engine.hpp"
#	include "Modules/Surface.hpp"
#	include "Modules/Scheme.hpp"

#	define WATERMARK_MSG_HEADER "Activate SAR"
#	define WATERMARK_MSG_HELPTEXT "Development SAR build. Do not use."

class WatermarkHud : public Hud {
public:
	WatermarkHud()
		: Hud(HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen, false) {
	}

	bool ShouldDraw() override {
		return !engine->demoplayer->IsPlaying() && !(networkManager.isConnected && networkManager.spectator);
	}

	bool GetCurrentSize(int &w, int &h) override {
		return false;
	}

	void Paint(int slot) override {
		int screenWidth, screenHeight;
		engine->GetScreenSize(nullptr, screenWidth, screenHeight);

		static Surface::HFont headerFont = scheme->FindFont("Trebuchet MS", 72);
		static Surface::HFont subTextFont = scheme->FindFont("Trebuchet MS", 24);
		if (headerFont == scheme->GetDefaultFont() || subTextFont == scheme->GetDefaultFont()) {
			static bool printedWarning = false;
			if (!printedWarning) { // don't spam the console / expensive-ish FindFont calls
				if (headerFont == scheme->GetDefaultFont()) headerFont = scheme->FindFont("DejaVu Sans", 72);
				if (subTextFont == scheme->GetDefaultFont()) subTextFont = scheme->FindFont("DejaVu Sans", 24);
				if (headerFont == scheme->GetDefaultFont() || subTextFont == scheme->GetDefaultFont()) {
					console->DevWarning("Failed to load font for watermark\n");
					printedWarning = true;
				}
			}
		}

		int fontSize = surface->GetFontHeight(headerFont);

		int xPos = screenWidth - std::max(surface->GetFontLength(headerFont, "%s", WATERMARK_MSG_HEADER), surface->GetFontLength(subTextFont, "%s", WATERMARK_MSG_HELPTEXT)) - fontSize * 2;

		surface->DrawTxt(headerFont, xPos, screenHeight - fontSize * 3, Color{255, 255, 255, 100}, "%s", WATERMARK_MSG_HEADER);
		surface->DrawTxt(subTextFont, xPos, screenHeight - fontSize * 2, Color{255, 255, 255, 100}, "%s", WATERMARK_MSG_HELPTEXT);
	}
};

WatermarkHud watermark;

#endif
