#include "InputHud.hpp"

#include <cstring>

#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"

#include "Utils/SDK.hpp"

#include "Variable.hpp"

Variable sar_ihud("sar_ihud", "0", 0, "Draws keyboard events of client.\n"
                                      "0 = Default,\n"
                                      "1 = forward;back;moveleft;moveright,\n"
                                      "2 = 1 + duck;jump;use,\n"
                                      "3 = 2 + attack;attack2,\n"
                                      "4 = 3 + speed;reload.\n");
Variable sar_ihud_x("sar_ihud_x", "0", 0, "X offset of input HUD.\n");
Variable sar_ihud_y("sar_ihud_y", "0", 0, "Y offset of input HUD.\n");
Variable sar_ihud_button_padding("sar_ihud_button_padding", "2", 0, "Padding between buttons of input HUD.\n");
Variable sar_ihud_button_size("sar_ihud_button_size", "60", 0, "Button size of input HUD.\n");
Variable sar_ihud_button_color("sar_ihud_button_color", "0 0 0 233", "RGBA button color of input HUD.\n", 0);
Variable sar_ihud_font_color("sar_ihud_font_color", "255 255 255 255", "RGBA font color of input HUD.\n", 0);
Variable sar_ihud_font_index("sar_ihud_font_index", "1", 0, "Font index of input HUD.\n");
Variable sar_ihud_layout("sar_ihud_layout", "WASDCSELRSR", "Layout of input HUD.\n"
                                                           "Characters are in this order:\n"
                                                           "forward,\n"                                          
                                                           "moveleft,\n"
                                                           "back,\n"
                                                           "moveright,\n"
                                                           "duck,\n"
                                                           "jump,\n"
                                                           "use,\n"
                                                           "attack,\n"
                                                           "attack2,\n"
                                                           "speed,\n"
                                                           "reload.\n"
                                                           "Keep it empty to disable drawing characters.\n",
    0);
Variable sar_ihud_shadow("sar_ihud_shadow", "1", "Draws button shadows of input HUD.\n");
Variable sar_ihud_shadow_color("sar_ihud_shadow_color", "0 0 0 32", "RGBA button shadow color of input HUD.\n", 0);
Variable sar_ihud_shadow_font_color("sar_ihud_shadow_font_color", "255 255 255 32", "RGBA button shadow font color of input HUD.\n", 0);

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

InputHud* inputHud;
InputHud* inputHud2;

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
        auto x = xOffset + (col * size) + ((col + 1) * padding);
        auto y = yOffset + (row * size) + ((row + 1) * padding);
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

    /*
        Layout:

            row|col0|1|2|3|4|5|6|7|8
            ---|---------------------
              0|       w|e|r
              1|shft|a|s|d
              2|ctrl|spacebar   |l|r
    */

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

int sar_ihud_setpos_CompletionFunc(const char* partial,
    char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    const char* cmd = "sar_ihud_setpos ";
    char* match = (char*)partial;
    if (std::strstr(partial, cmd) == partial) {
        match = match + std::strlen(cmd);
    }

    static auto positions = std::vector<std::string>{
        std::string("top left"),
        std::string("top center"),
        std::string("top right"),
        std::string("center left"),
        std::string("center center"),
        std::string("center right"),
        std::string("bottom left"),
        std::string("bottom center"),
        std::string("bottom right")
    };

    // Filter items
    static auto items = std::vector<std::string>(positions.size());
    items.clear();
    for (auto& pos : positions) {
        if (items.size() == COMMAND_COMPLETION_MAXITEMS) {
            break;
        }

        if (std::strlen(match) != std::strlen(cmd)) {
            if (std::strstr(pos.c_str(), match)) {
                items.push_back(pos);
            }
        } else {
            items.push_back(pos);
        }
    }

    // Copy items into list buffer
    auto count = 0;
    for (auto& item : items) {
        std::strcpy(commands[count++], (std::string(cmd) + item).c_str());
    }

    return count;
}
CON_COMMAND_F_COMPLETION(sar_ihud_setpos, "Sets automatically the position of input HUD.\n"
                                          "Usage: sar_ihud_setpos <top, center or bottom> <left, center or right>\n",
    0,
    sar_ihud_setpos_CompletionFunc)
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
#if _WIN32
    engine->GetScreenSize(xScreen, yScreen);
#else
    engine->GetScreenSize(nullptr, xScreen, yScreen);
#endif

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
