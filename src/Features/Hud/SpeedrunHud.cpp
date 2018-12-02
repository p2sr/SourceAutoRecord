#include "SpeedrunHud.hpp"

#include "Features/Speedrun/SpeedrunTimer.hpp"

#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"

#include "Variable.hpp"

Variable sar_sr_hud("sar_sr_hud", "0", 0, "Draws speedrun timer.\n");
Variable sar_sr_hud_x("sar_sr_hud_x", "0", 0, "X offset of speedrun timer HUD.\n");
Variable sar_sr_hud_y("sar_sr_hud_y", "100", 0, "Y offset of speedrun timer HUD.\n");
Variable sar_sr_hud_font_color("sar_sr_hud_font_color", "255 255 255 255", "RGBA font color of speedrun timer HUD.\n", 0);
Variable sar_sr_hud_font_index("sar_sr_hud_font_index", "70", 0, "Font index of speedrun timer HUD.\n");

SpeedrunHud* speedrunHud;

bool SpeedrunHud::GetCurrentSize(int& xSize, int& ySize)
{
    return false;
}
void SpeedrunHud::Draw()
{
    auto mode = sar_sr_hud.GetInt();
    if (mode == 0)
        return;

    surface->StartDrawing(surface->matsurface->ThisPtr());

    auto total = speedrun->GetTotal();
    auto ipt = speedrun->GetIntervalPerTick();

    auto xOffset = sar_sr_hud_x.GetInt();
    auto yOffset = sar_sr_hud_y.GetInt();

    auto font = scheme->GetDefaultFont() + sar_sr_hud_font_index.GetInt();
    auto fontColor = this->GetColor(sar_sr_hud_font_color.GetString());

    surface->DrawTxt(font, xOffset, yOffset, fontColor, "%s", SpeedrunTimer::Format(total * ipt).c_str());

    surface->FinishDrawing();
}
