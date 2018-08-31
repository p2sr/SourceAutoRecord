#pragma once
#include "Module.hpp"

#include "Features/Hud/InputHud.hpp"
#include "Features/Hud/SpeedrunHud.hpp"

#include "Interface.hpp"
#include "Utils.hpp"

class VGui : public Module {
public:
    Interface* enginevgui;

    InputHud* inputHud;
    SpeedrunHud* speedrunHud;
    bool respectClShowPos = false;

private:
    // CEngineVGui::Paint
    DECL_DETOUR(Paint, int mode)

public:
    bool Init() override;
    void Shutdown() override;
};

extern VGui* vgui;
