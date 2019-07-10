#pragma once
#include <vector>

#include "Module.hpp"

#include "Features/Hud/InputHud.hpp"
#include "Features/Hud/InspectionHud.hpp"
#include "Features/Hud/SpeedrunHud.hpp"

#include "Interface.hpp"
#include "Utils.hpp"

class VGui : public Module {
public:
    Interface* enginevgui = nullptr;

private:
    std::vector<Hud*> huds = std::vector<Hud*>();
    std::vector<Hud*> huds2 = std::vector<Hud*>();

    bool respectClShowPos = true;

public:
    // CEngineVGui::Paint
    DECL_DETOUR(Paint, int mode);

    bool Init() override;
    void Shutdown() override;
    const char* Name() override { return MODULE("engine"); }
};

extern VGui* vgui;
