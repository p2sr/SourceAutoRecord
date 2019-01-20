#pragma once
#include "Hud.hpp"

#include "Variable.hpp"

class InputHud : public Hud {
private:
    int buttonBits = 0;

public:
    void SetButtonBits(int buttonBits);
    bool GetCurrentSize(int& xSize, int& ySize) override;
    void Draw() override;
};

extern InputHud* inputHud;
extern InputHud* inputHud2;

extern Variable sar_ihud;
extern Variable sar_ihud_x;
extern Variable sar_ihud_y;
extern Variable sar_ihud_button_padding;
extern Variable sar_ihud_button_size;
extern Variable sar_ihud_button_color;
extern Variable sar_ihud_font_color;
extern Variable sar_ihud_font_index;
extern Variable sar_ihud_layout;
extern Variable sar_ihud_shadow;
extern Variable sar_ihud_shadow_color;
extern Variable sar_ihud_shadow_font_color;
