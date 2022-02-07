#include "SpeedrunHud.hpp"

#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Variable.hpp"

Variable sar_sr_hud("sar_sr_hud", "0", 0, "Draws speedrun timer.\n");
Variable sar_sr_hud_x("sar_sr_hud_x", "0", "X offset of speedrun timer HUD.\n", 0);
Variable sar_sr_hud_y("sar_sr_hud_y", "100", "Y offset of speedrun timer HUD.\n", 0);
Variable sar_sr_hud_font_color("sar_sr_hud_font_color", "255 255 255 255", "RGBA font color of speedrun timer HUD.\n", 0);
Variable sar_sr_hud_font_index("sar_sr_hud_font_index", "70", 0, "Font index of speedrun timer HUD.\n");

SpeedrunHud speedrunHud;

SpeedrunHud::SpeedrunHud()
	: Hud(HudType_InGame | HudType_Menu | HudType_Paused | HudType_LoadingScreen, true) {
}
bool SpeedrunHud::ShouldDraw() {
	return sar_sr_hud.GetBool() && Hud::ShouldDraw();
}
void SpeedrunHud::Paint(int slot) {
	auto total = SpeedrunTimer::GetTotalTicks();
	auto ipt = *engine->interval_per_tick;

	auto xOffset = sar_sr_hud_x.GetInt();
	auto yOffset = sar_sr_hud_y.GetInt();

	auto font = scheme->GetFontByID(sar_sr_hud_font_index.GetInt());
	auto fontColor = Utils::GetColor(sar_sr_hud_font_color.GetString(),false).value_or(Color(255,255,255,255));

	auto text = SpeedrunTimer::Format(total * ipt);

	auto width = surface->GetFontLength(font, "%s", text.c_str());
	auto height = surface->GetFontHeight(font);

	int sw, sh;
	engine->GetScreenSize(nullptr, sw, sh);

	if (xOffset < 0) xOffset += sw - width;
	if (yOffset < 0) yOffset += sh - height;

	surface->DrawTxt(font, xOffset, yOffset, fontColor, "%s", text.c_str());
}
bool SpeedrunHud::GetCurrentSize(int &xSize, int &ySize) {
	return false;
}
