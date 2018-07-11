#pragma once
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"

#include "Cheats.hpp"
#include "Game.hpp"

#define IN_ATTACK (1 << 0)
#define IN_JUMP (1 << 1)
#define IN_DUCK (1 << 2)
#define IN_FORWARD (1 << 3)
#define IN_BACK (1 << 4)
#define IN_USE (1 << 5)
#define IN_MOVELEFT (1 << 9)
#define IN_MOVERIGHT (1 << 10)
#define IN_ATTACK2 (1 << 11)
#define IN_RELOAD (1 << 13)
#define IN_SPEED (1 << 17)

namespace InputHud {

int ButtonBits = 0;
int FontIndexOffset = 0;

/*
    Layout:

              row|col0|1|2|3|4|5|6|7|8
              ---|---------------------
                0|       w|e|r
                1|shft|a|s|d
                2|ctrl|spacebar   |l|r
*/

const int row0 = 0;
const int row1 = 1;
const int row2 = 2;
const int col0 = 0;
const int col1 = 1;
const int col2 = 2;
const int col3 = 3;
const int col4 = 4;
const int col5 = 5;
const int col6 = 6;
const int col7 = 7;
const int col8 = 8;
const int spacebarSize = 6;

void Init()
{
#ifndef _WIN32
    if (Game::Version == Game::Portal2) {
        FontIndexOffset = 1;
    }
#endif
}
void SetButtonBits(int bits)
{
    ButtonBits = bits;
}
Color GetColor(const char* source)
{
    int r, g, b, a;
    sscanf_s(source, "%i%i%i%i", &r, &g, &b, &a);
    return Color(r, g, b, a);
}
void Draw()
{
    auto mode = Cheats::sar_ihud.GetInt();
    if (mode == 0)
        return;

    auto mvForward = ButtonBits & IN_FORWARD;
    auto mvBack = ButtonBits & IN_BACK;
    auto mvLeft = ButtonBits & IN_MOVELEFT;
    auto mvRight = ButtonBits & IN_MOVERIGHT;
    auto mvJump = ButtonBits & IN_JUMP;
    auto mvDuck = ButtonBits & IN_DUCK;
    auto mvUse = ButtonBits & IN_USE;
    auto mvAttack = ButtonBits & IN_ATTACK;
    auto mvAttack2 = ButtonBits & IN_ATTACK2;
    auto mvReload = ButtonBits & IN_RELOAD;
    auto mvSpeed = ButtonBits & IN_SPEED;

    Surface::StartDrawing(Surface::matsurface->GetThisPtr());

    auto xOffset = Cheats::sar_ihud_x.GetInt();
    auto yOffset = Cheats::sar_ihud_y.GetInt();
    auto size = Cheats::sar_ihud_size.GetInt();
    auto padding = Cheats::sar_ihud_padding.GetInt();

    auto color = GetColor(Cheats::sar_ihud_color.GetString());
    auto fontColor = GetColor(Cheats::sar_ihud_font_color.GetString());
    auto font = Scheme::GetDefaultFont() + (int)Cheats::sar_ihud_font_index.GetFloat() - FontIndexOffset;

    auto symbols = std::string("WASDCSELRSR");
    auto layout = std::string(Cheats::sar_ihud_layout.GetString());
    if (layout.length() == symbols.length()) {
        symbols = layout;
    } else if (layout.length() == 0) {
        symbols = "           ";
    }

    auto shadow = Cheats::sar_ihud_shadow.GetBool();
    auto shadowColor = GetColor(Cheats::sar_ihud_shadow_color.GetString());
    auto shadowFontColor = GetColor(Cheats::sar_ihud_shadow_font_color.GetString());

    auto element = 0;
    auto DrawElement = [xOffset, yOffset, mode, shadow, color, size, shadowColor, font, fontColor, shadowFontColor, padding, symbols,
        &element](int value, bool button, int col, int row, int xFactor = 1)
    {
        int x = xOffset + (col * size);
        int y = yOffset + (row * size);
        if (mode >= value && (button || shadow)) {
            Surface::DrawRect((button) ? color : shadowColor,
                x + ((col + 1) * padding),
                y + ((row + 1) * padding),
                x + ((col + xFactor) * padding) + (xFactor * size),
                y + ((row + 1) * padding + size),
                font,
                (button) ? fontColor : shadowFontColor,
                "%c",
                symbols[element]);
        }
        element++;
    };

    DrawElement(1, mvForward, col2, row0);
    DrawElement(1, mvLeft, col1, row1);
    DrawElement(1, mvBack, col2, row1);
    DrawElement(1, mvRight, col3, row1);

    DrawElement(2, mvDuck, col0, row2);
    DrawElement(2, mvJump, col1, row2, spacebarSize);
    DrawElement(2, mvUse, col3, row0);

    DrawElement(3, mvAttack, col7, row2);
    DrawElement(3, mvAttack2, col8, row2);

    DrawElement(4, mvSpeed, col0, row1);
    DrawElement(4, mvReload, col4, row0);

    Surface::FinishDrawing();
}
}