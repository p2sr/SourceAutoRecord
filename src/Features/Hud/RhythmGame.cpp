#include "RhythmGame.hpp"

#include "Event.hpp"
#include "Variable.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Client.hpp"
#include "Modules/Surface.hpp"
#include "Features/GroundFramesCounter.hpp"
#include "Features/Session.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Console.hpp"

Variable sar_rhythmgame("sar_rhythmgame", "0", "Show a HUD indicating your groundframes as rhythm game like popups.\n");

bool RhythmGameHud::ShouldDraw() {
	return sar_rhythmgame.GetBool();
}

int RhythmGameHud::GetGroundframes() {
	return groundframes;
}

bool prevGrounded = false;
void RhythmGameHud::HandleGroundframeLogic(int slot, bool grounded) {
	if (!prevGrounded && grounded) {
		rhythmGameHud.groundframes = 0;
	} else if (grounded) {
		rhythmGameHud.groundframes++;
	}

	prevGrounded = grounded;
}

void RhythmGameHud::Paint(int slot) {
	int screenWidth, screenHeight;
	engine->GetScreenSize(nullptr, screenWidth, screenHeight);
	auto font = scheme->GetFontByID(49);
	float fh = surface->GetFontHeight(font);

	for (int i = 0; i < popups.size(); i++) {
		RhythmGamePopup popup = popups[i];

		int x = popup.x;
		int y = popup.y;

		Color popupColor;
		std::string popupText;
		if (popup.type == 0) {
			popupColor = perfectColor;
			popupColor.a = popup.alpha;

			popupText = "PERFECT";
		} else if (popup.type == 1) {
			popupColor = goodColor;
			popupColor.a = popup.alpha;

			popupText = "GOOD";
		} else if (popup.type == 2) {
			popupColor = okColor;
			popupColor.a = popup.alpha;

			popupText = "OK";
		} else if (popup.type == 3) {
			popupColor = badColor;
			popupColor.a = popup.alpha;

			popupText = "BAD";
		} else {
			return;
		}

		surface->DrawTxt(font, x, y, popupColor, popupText.c_str());
		if (popup.streak > 1 && popup.type == 0) {
			std::string combo = "(" + std::to_string(popup.streak) + "x)";
			surface->DrawTxt(font, x + (surface->GetFontLength(font, popupText.c_str()) / 2) - (surface->GetFontLength(font, combo.c_str()) / 2), y + fh, Color{232, 179, 45, static_cast<uint8_t>(popup.alpha)}, combo.c_str());
		}

		popups[i].lifetime--;
		popups[i].alpha -= 255 / 180;
		popups[i].y -= 2;

		if (popups[i].lifetime <= 0) {
			popups.erase(popups.begin() + i);
			i--;
		}
	}
}

void RhythmGameHud::OnJump(int slot) {
	int tick = session->GetTick();
	int groundframes = GetGroundframes();

	int screenWidth, screenHeight;
	engine->GetScreenSize(nullptr, screenWidth, screenHeight);

	auto font = scheme->GetFontByID(49);

	int x = Math::RandomNumber(0, screenWidth - surface->GetFontLength(font, "PERFECT"));
	int y = screenHeight - 200;

	RhythmGamePopup popup;
	popup.x = x;
	popup.y = y;
	popup.alpha = 255;
	popup.lifetime = 180;
	popup.type = -1;
	popup.streak = 0;

	if (groundframes == 0) {
		popup.type = 0;
	} else if (groundframes == 1) {
		popup.type = 1;
	} else if (groundframes == 2) {
		popup.type = 2;
	} else if (groundframes >= 3 && groundframes <= 6) {
		popup.type = 3;
	} else {
		perfectsInARow = 0;
		return;
	}

	if (popup.type == 0) {
		perfectsInARow++;
	} else {
		perfectsInARow = 0;
	}

	popup.streak = perfectsInARow;

	popups.push_back(popup);
}

RhythmGameHud rhythmGameHud;