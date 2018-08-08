#pragma once
#include "Hud.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"

#include "Cheats.hpp"

#include "Utils/SDK.hpp"

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

class InputHud : public Hud {
private:
    int buttonBits = 0;

public:
    void SetButtonBits(int buttonBits);
    bool GetCurrentSize(int& xSize, int& ySize) override;
    void Draw() override;
};

InputHud* inputHud;
extern InputHud* inputHud;

void InputHud::SetButtonBits(int buttonBits)
{
    this->buttonBits = buttonBits;
}
bool InputHud::GetCurrentSize(int& xSize, int& ySize)
{
    auto mode = sar_ihud.GetInt();
    if (mode == 0) {
        return false;
    }

    auto size = sar_ihud_button_size.GetInt();
    auto padding = sar_ihud_button_padding.GetInt();

    switch (mode) {
    case 1:
        xSize = (col4 + 1) * (size + (2 * padding));
        ySize = (row1 + 1) * (size + (2 * padding));
        break;
    case 2:
        xSize = (col6 + 1) * (size + (2 * padding));
        ySize = (row2 + 1) * (size + (2 * padding));
        break;
    default:
        xSize = (col8 + 1) * (size + (2 * padding));
        ySize = (row2 + 1) * (size + (2 * padding));
        break;
    }

    return true;
}
void InputHud::Draw()
{
    auto mode = sar_ihud.GetInt();
    if (mode == 0)
        return;

    auto mvForward = this->buttonBits & IN_FORWARD;
    auto mvBack = this->buttonBits & IN_BACK;
    auto mvLeft = this->buttonBits & IN_MOVELEFT;
    auto mvRight = this->buttonBits & IN_MOVERIGHT;
    auto mvJump = this->buttonBits & IN_JUMP;
    auto mvDuck = this->buttonBits & IN_DUCK;
    auto mvUse = this->buttonBits & IN_USE;
    auto mvAttack = this->buttonBits & IN_ATTACK;
    auto mvAttack2 = this->buttonBits & IN_ATTACK2;
    auto mvReload = this->buttonBits & IN_RELOAD;
    auto mvSpeed = this->buttonBits & IN_SPEED;

    surface->StartDrawing(surface->matsurface->ThisPtr());

    auto xOffset = sar_ihud_x.GetInt();
    auto yOffset = sar_ihud_y.GetInt();
    auto size = sar_ihud_button_size.GetInt();
    auto padding = sar_ihud_button_padding.GetInt();

    auto color = this->GetColor(sar_ihud_button_color.GetString());
    auto fontColor = this->GetColor(sar_ihud_font_color.GetString());
    auto font = scheme->GetDefaultFont() + sar_ihud_font_index.GetInt();

    auto symbols = std::string("WASDCSELRSR");
    auto layout = std::string(sar_ihud_layout.GetString());
    if (layout.length() == symbols.length()) {
        symbols = layout;
    } else if (layout.length() == 0) {
        symbols = "           ";
    }

    auto shadow = sar_ihud_shadow.GetBool();
    auto shadowColor = this->GetColor(sar_ihud_shadow_color.GetString());
    auto shadowFontColor = this->GetColor(sar_ihud_shadow_font_color.GetString());

    auto element = 0;
    auto DrawElement = [xOffset, yOffset, mode, shadow, color, size, shadowColor, font, fontColor, shadowFontColor, padding, symbols,
                           &element](int value, bool button, int col, int row, int length = 1) {
        int x = xOffset + (col * size) + ((col + 1) * padding);
        int y = yOffset + (row * size) + ((row + 1) * padding);
        if (mode >= value && (button || shadow)) {
            surface->DrawRectAndCenterTxt((button) ? color : shadowColor,
                x + ((col + 1) * padding),
                y + ((row + 1) * padding),
                x + ((((col + 1) * padding) + size) * length),
                y + ((row + 1) * padding + size),
                font,
                (button) ? fontColor : shadowFontColor,
                "%c",
                symbols[element]);
        }
        ++element;
    };

    DrawElement(1, mvForward, col2, row0);
    DrawElement(1, mvLeft, col1, row1);
    DrawElement(1, mvBack, col2, row1);
    DrawElement(1, mvRight, col3, row1);

    DrawElement(2, mvDuck, col0, row2);
    DrawElement(2, mvJump, col1, row2, col6);
    DrawElement(2, mvUse, col3, row0);

    DrawElement(3, mvAttack, col7, row2);
    DrawElement(3, mvAttack2, col8, row2);

    DrawElement(4, mvSpeed, col0, row1);
    DrawElement(4, mvReload, col4, row0);

    surface->FinishDrawing();
}

CON_COMMAND(sar_ihud_setpos, "Sets automatically the position of input HUD. "
                             "Usage: sar_ihud_setpos <top, center or bottom> <left, center or right>\n")
{
    if (args.ArgC() != 3) {
        console->Print("sar_ihud_setpos <top, center or bottom> <left, center or right> : "
                       "Sets automatically the position of input HUD.\n");
        return;
    }

    auto xSize = 0;
    auto ySize = 0;

    if (!inputHud->GetCurrentSize(xSize, ySize)) {
        console->Print("HUD not active!\n");
        return;
    }

    auto xScreen = 0;
    auto yScreen = 0;
    Engine::GetScreenSize(xScreen, yScreen);

    auto xPos = sar_ihud_x.GetInt();
    auto yPos = sar_ihud_y.GetInt();

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

    sar_ihud_x.SetValue(xPos);
    sar_ihud_y.SetValue(yPos);
}
