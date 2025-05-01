#include "SpeedrunHud.hpp"

#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Variable.hpp"

Variable sar_sr_hud("sar_sr_hud", "0", 0, "Draws speedrun timer. 1 = speedrun timer, 2 = speedrun and split timer.\n", FCVAR_DONTRECORD);
Variable sar_sr_hud_x("sar_sr_hud_x", "0", "X offset of speedrun timer HUD.\n", FCVAR_DONTRECORD);
Variable sar_sr_hud_y("sar_sr_hud_y", "100", "Y offset of speedrun timer HUD.\n", FCVAR_DONTRECORD);
Variable sar_sr_hud_font_color("sar_sr_hud_font_color", "255 255 255 255", "RGBA font color of speedrun timer HUD.\n", FCVAR_DONTRECORD);
Variable sar_sr_hud_font_index("sar_sr_hud_font_index", "70", 0, "Font index of speedrun timer HUD.\n", FCVAR_DONTRECORD);
Variable sar_sr_hud_font_index_2("sar_sr_hud_font_index_2", "71", 0, "Font index of speedrun split timer HUD.\n", FCVAR_DONTRECORD);
Variable sar_sr_hud_bg("sar_sr_hud_bg", "0", "Draw a background behind the speedrun timer.\n", FCVAR_DONTRECORD);

SpeedrunHud speedrunHud;

SpeedrunHud::SpeedrunHud()
	: Hud(HudType_InGame | HudType_Menu | HudType_Paused | HudType_LoadingScreen, true) {
}
bool SpeedrunHud::ShouldDraw() {
	return sar_sr_hud.GetBool() && Hud::ShouldDraw();
}
void SpeedrunHud::Paint(int slot) {
	auto total = SpeedrunTimer::GetTotalTicks();
	auto split = SpeedrunTimer::GetSplitTicks();
	auto ipt = engine->GetIPT();

	auto xOffset = sar_sr_hud_x.GetInt();
	auto yOffset = sar_sr_hud_y.GetInt();

	auto font = scheme->GetFontByID(sar_sr_hud_font_index.GetInt());
	auto font2 = scheme->GetFontByID(sar_sr_hud_font_index_2.GetInt());
	auto fontColor = Utils::GetColor(sar_sr_hud_font_color.GetString(), false).value_or(Color(255, 255, 255, 255));

	auto text = SpeedrunTimer::Format(total * ipt);
	auto text2 = SpeedrunTimer::Format(split * ipt);

	auto width1 = surface->GetFontLength(font, "%s", text.c_str());
	auto width2 = surface->GetFontLength(font2, "%s", text2.c_str());
	auto height1 = surface->GetFontHeight(font);
	auto height2 = surface->GetFontHeight(font2);

	auto width = width1;
	auto height = height1;

	if (sar_sr_hud.GetInt() >= 2) {
		width = std::max(width1, width2);
		height = height1 + height2;
	}

	int sw, sh;
	engine->GetScreenSize(nullptr, sw, sh);

	if (xOffset < 0) xOffset += sw - width;
	if (yOffset < 0) yOffset += sh - height;

	if (sar_sr_hud_bg.GetBool()) {
		surface->DrawRect(Color{0, 0, 0, 192}, xOffset - 2, yOffset - 2, xOffset + width + 2, yOffset + height + 2);
	}

	surface->DrawTxt(font, xOffset + width - width1, yOffset, fontColor, "%s", text.c_str());

	if (sar_sr_hud.GetInt() >= 2) {
		yOffset += height1;
		surface->DrawTxt(font2, xOffset + width - width2, yOffset, fontColor, "%s", text2.c_str());
	}
}
bool SpeedrunHud::GetCurrentSize(int &xSize, int &ySize) {
	return false;
}
