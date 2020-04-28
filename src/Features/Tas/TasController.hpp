#pragma once

#include "Features/Feature.hpp"

#include "Utils/SDK.hpp"

#include "Command.hpp"
#include "Variable.hpp"

struct TasControllerAnalog {
    float x;
    float y;
};

class TasController : public Feature {
public:
    TasControllerAnalog moveAnalog;
    TasControllerAnalog viewAnalog;
    int buttons;
    int oldButtons;
public:
    TasController();
    ~TasController();

    void ControllerMove(int nSlot, float flFrametime, CUserCmd* cmd);
};

extern TasController* tasController;
