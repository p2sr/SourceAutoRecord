#include "TasControllerHud.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Variable.hpp"

#include "Features/Tas/TasController.hpp"

#include <algorithm>

TasControllerHud tasControllerHud;

Variable sar_tas_controller_hud("sar_tas_controller_hud", "0", "Enables or disables the TAS controller HUD on screen.\n");
Variable sar_tas_controller_hud_x("sar_tas_controller_hud_x", "10", -99999, 99999, "Sets the X position of the TAS controller HUD.\n");
Variable sar_tas_controller_hud_y("sar_tas_controller_hud_y", "-10", -99999, 99999, "Sets the Y position of the TAS controller HUD.\n");
Variable sar_tas_controller_hud_bg("sar_tas_controller_hud_bg", "0 0 0 192", "RGBA background cololr of the TAS controller HUD.\n", 0);

TasControllerHud::TasControllerHud()
	: Hud(HudType_InGame, true) {
}
bool TasControllerHud::ShouldDraw() {
	bool shouldDraw = sar_tas_controller_hud.GetBool() && Hud::ShouldDraw();
	return shouldDraw;
}

void TasControllerHud::Paint(int slot) {
	// update angles array
	if (queryNewAngles) {
		this->angles[1] = this->angles[0];
		this->angles[0] = engine->GetAngles(GET_SLOT());
		queryNewAngles = false;
	}

	// do the actual drawing
	auto font = scheme->GetDefaultFont() + 3;

	int cX = sar_tas_controller_hud_x.GetInt();
	int cY = sar_tas_controller_hud_y.GetInt();

	int xScreen, yScreen;
#if _WIN32
	engine->GetScreenSize(xScreen, yScreen);
#else
	engine->GetScreenSize(nullptr, xScreen, yScreen);
#endif

	const int joystickSize = 200;
	const int joystickPadding = 10;
	const int vertPadding = 30;

	int bgWidth = joystickSize * 2 + joystickPadding * 4;
	int bgHeight = joystickSize + joystickPadding * 2 + vertPadding * 2;

	if (cX < 0)
		cX = xScreen + cX - bgWidth;
	if (cY < 0)
		cY = yScreen + cY - bgHeight;

	int bgR, bgG, bgB, bgA;
	sscanf(sar_tas_controller_hud_bg.GetString(), "%i%i%i%i", &bgR, &bgG, &bgB, &bgA);
	surface->DrawRect(Color(bgR, bgG, bgB, bgA), cX, cY, cX + bgWidth, cY + bgHeight);

	for (int i = 0; i < 2; i++) {

		int jX = cX + joystickPadding * (2*i + 1) + joystickSize * i;
		int jY = cY + joystickPadding + vertPadding;

		int r = joystickSize/2;

		surface->DrawColoredLine(jX, jY, jX + joystickSize, jY, Color(192, 192, 192, 128));
		surface->DrawColoredLine(jX, jY, jX, jY + joystickSize, Color(192, 192, 192, 128));
		surface->DrawColoredLine(jX + joystickSize, jY, jX + joystickSize, jY + joystickSize, Color(192, 192, 192, 128));
		surface->DrawColoredLine(jX, jY + joystickSize, jX + joystickSize, jY + joystickSize, Color(192, 192, 192, 128));

		surface->DrawFilledCircle(jX + r, jY + r, r, Color(0, 0, 0, 128));
		surface->DrawCircle(jX + r, jY + r, r, Color(192, 192, 192, 128));

		surface->DrawColoredLine(jX, jY + r, jX + joystickSize, jY + r, Color(192, 192, 192, 64));
		surface->DrawColoredLine(jX + r, jY, jX + r, jY + joystickSize, Color(192, 192, 192, 64));

		Vector v, visV;
		if (i == 0) {
			// recalculate movement values into controller inputs
			v = movement;
			v.y /= cl_forwardspeed.GetFloat();
			v.x /= cl_sidespeed.GetFloat();
			visV = v;
			visV.y *= -1;
		} else {
			// calculating the difference between angles in two frames
			v = {
				angles[1].y - angles[0].y,
				angles[1].x - angles[0].x,
			};

			while (v.x < -180.0f) v.x += 360.0f;
			if (v.x > 180.0f) v.x -= 360.0f;

			// make lower range of inputs easier to notice
			if (v.Length() > 0) {
				visV = v.Normalize() * pow(v.Length() / 180.0f, 0.2);
				visV.y *= -1;
			}
		}

		Color pointerColor = (i == 0) ? Color(255, 150, 0) : Color(0, 150, 255);
		Vector pointerPoint = {jX + r + r * visV.x, jY + r + r * visV.y};
		surface->DrawColoredLine(jX + r, jY + r, pointerPoint.x, pointerPoint.y, pointerColor);
		surface->DrawFilledCircle(pointerPoint.x, pointerPoint.y, 10, pointerColor);

		Color textColor = Color(255, 255, 255, 255);
		surface->DrawTxt(font, jX, jY + joystickSize + 15, textColor, "x:%.3f", v.x);
		surface->DrawTxt(font, jX + r, jY + joystickSize + 15, textColor, "y:%.3f", v.y);

		surface->DrawTxt(font, jX, jY - 20, textColor, i == 0 ? "movement" : "angles", v.x);
	}
}

bool TasControllerHud::GetCurrentSize(int &xSize, int &ySize) {
	return false;
}

void TasControllerHud::AddData(Vector movement) {
	this->movement = movement;
	queryNewAngles = true;
}
