#pragma once
#include <vector>

#include "Features/Hud/Hud.hpp"

#include "Module.hpp"

#include "Interface.hpp"
#include "Utils.hpp"

class VGui : public Module {
public:
    Interface* enginevgui = nullptr;

private:
    HudContext context = HudContext();
    std::vector<Hud*> huds = std::vector<Hud*>();

public:
    std::vector<HudElement*> elements = std::vector<HudElement*>();

private:
    void Draw(Hud* const& hud);
    void Draw(HudElement* const& element);

public:

    using _IsGameUIVisible = bool(__rescall*)(void* thisptr);

    _IsGameUIVisible IsGameUIVisible = nullptr;

    bool IsUIVisible();

    // CEngineVGui::Paint
    DECL_DETOUR(Paint, PaintMode_t mode);

    bool Init() override;
    void Shutdown() override;
    const char* Name() override { return MODULE("engine"); }
};

extern VGui* vgui;
