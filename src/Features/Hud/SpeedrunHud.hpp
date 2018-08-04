#pragma once
#include "Hud.hpp"

#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"

#include "Features/Speedrun.hpp"

#include "Cheats.hpp"

class SpeedrunHud : public Hud {
public:
    bool GetCurrentSize(int& xSize, int& ySize);
    void Draw();
};

SpeedrunHud* speedrunHud;
extern SpeedrunHud* speedrunHud;

bool SpeedrunHud::GetCurrentSize(int& xSize, int& ySize)
{
    return false;
}
void SpeedrunHud::Draw()
{
    auto mode = sar_sr_hud.GetInt();
    if (mode == 0)
        return;

    Surface::StartDrawing(Surface::matsurface->ThisPtr());

    auto total = Speedrun::timer->GetTotal();
    auto ipt = Speedrun::timer->GetIntervalPerTick();

    auto xOffset = sar_sr_hud_x.GetInt();
    auto yOffset = sar_sr_hud_y.GetInt();

    auto font = Scheme::GetDefaultFont() + sar_sr_hud_font_index.GetInt();
    auto fontColor = this->GetColor(sar_sr_hud_font_color.GetString());

    Surface::DrawTxt(font, xOffset, yOffset, fontColor, "%s", Speedrun::Timer::Format(total * ipt).c_str());

    Surface::FinishDrawing();
}
