#pragma once
#include "Utils/SDK.hpp"

class Hud {
protected:
    Color GetColor(const char* source);

public:
    virtual ~Hud() = default;
    virtual bool GetCurrentSize(int& xSize, int& ySize) = 0;
    virtual void Draw() = 0;
};
