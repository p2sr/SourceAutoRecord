#pragma once
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"

#include "Features/Speedrun.hpp"

#include "Cheats.hpp"
#include "Command.hpp"
#include "Game.hpp"

namespace SpeedrunHud {

Color GetColor(const char* source)
{
    int r, g, b, a;
    sscanf(source, "%i%i%i%i", &r, &g, &b, &a);
    return Color(r, g, b, a);
}
bool GetCurrentSize(int& xSize, int& ySize)
{
    auto mode = Cheats::sar_ihud.GetInt();
    if (mode == 0) {
        return false;
    }

    return true;
}
void Draw()
{
    auto mode = Cheats::sar_sr_hud.GetInt();
    if (mode == 0)
        return;

    Surface::StartDrawing(Surface::matsurface->GetThisPtr());

    auto total = Speedrun::timer->GetTotal();
    auto ipt = Speedrun::timer->GetIntervalPerTick();

    auto xOffset = Cheats::sar_sr_hud_x.GetInt();
    auto yOffset = Cheats::sar_sr_hud_y.GetInt();
    auto color = GetColor(Cheats::sar_sr_hud_color.GetString());
    auto fontColor = GetColor(Cheats::sar_sr_hud_font_color.GetString());

    auto size = Cheats::sar_sr_hud_size.GetInt();
    auto font = Scheme::GetDefaultFont() + Cheats::sar_sr_hud_font_index.GetInt();

    /*Surface::DrawRect(color,
        xOffset,
        yOffset,
        xOffset + (size * 4),
        yOffset + size);*/
    Surface::DrawTxt(Scheme::GetDefaultFont(),
        xOffset,
        yOffset,
        fontColor,
        "%s", Speedrun::Timer::Format(total * ipt).c_str());

    Surface::FinishDrawing();
}
}

CON_COMMAND(sar_sr_hud_setpos, "Sets automatically the position of speedrun timer HUD. "
                               "Usage: sar_sr_hud_setpos <top, center or bottom>-<left, center or right>\n")
{
    if (args.ArgC() != 3) {
        console->Print("sar_sr_hud_setpos <top, center or bottom>-<left, center or right> : "
                       "Sets automatically the position of input HUD.\n");
        return;
    }

    auto xSize = 0;
    auto ySize = 0;
    if (!SpeedrunHud::GetCurrentSize(xSize, ySize)) {
        console->Print("HUD not active!\n");
        return;
    }

    auto xScreen = 0;
    auto yScreen = 0;
    Engine::GetScreenSize(xScreen, yScreen);

    auto xPos = Cheats::sar_sr_hud_x.GetInt();
    auto yPos = Cheats::sar_sr_hud_y.GetInt();

    if (!std::strcmp(args[1], "top")) {
        yPos = 0;
    } else if (!std::strcmp(args[1], "center")) {
        yPos = (yScreen / 2) - (ySize / 2);
    } else if (!std::strcmp(args[1], "bottom")) {
        yPos = yScreen - ySize;
    }

    if (!std::strcmp(args[2], "left")) {
        xPos = 0;
    } else if (!std::strcmp(args[2], "center")) {
        xPos = (xScreen / 2) - (xSize / 2);
    } else if (!std::strcmp(args[2], "right")) {
        xPos = xScreen - xSize;
    }

    Cheats::sar_sr_hud_x.SetValue(xPos);
    Cheats::sar_sr_hud_y.SetValue(yPos);
}
