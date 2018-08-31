#pragma once
#include "Hud.hpp"

class SpeedrunHud : public Hud {
public:
    bool GetCurrentSize(int& xSize, int& ySize) override;
    void Draw() override;
};
