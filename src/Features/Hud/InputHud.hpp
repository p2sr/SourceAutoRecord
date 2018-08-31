#pragma once
#include "Hud.hpp"

class InputHud : public Hud {
private:
    int buttonBits = 0;

public:
    void SetButtonBits(int buttonBits);
    bool GetCurrentSize(int& xSize, int& ySize) override;
    void Draw() override;
};
