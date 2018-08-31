#include "SpeedrunHud.hpp"

#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"

#include "Features/Speedrun/SpeedrunTimer.hpp"

#include "Cheats.hpp"

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
