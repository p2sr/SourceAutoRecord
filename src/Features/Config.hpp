#pragma once
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Feature.hpp"

#include "Cheats.hpp"
#include "Utils.hpp"

#define SAVE_CVAR(cvar, value) \
    file << #cvar " " << cvar.Get##value() << "\n";

class Config : public Feature {
public:
    std::string filePath;

public:
    Config();
    bool Save();
    bool Load();
};

Config::Config()
    : filePath("/cfg/_sar_cvars.cfg")
{
    this->hasLoaded = true;
}
bool Config::Save()
{
    if (Engine::GetGameDirectory == nullptr)
        return false;

    std::ofstream file(std::string(Engine::GetGameDirectory()) + this->filePath, std::ios::out | std::ios::trunc);
    if (!file.good())
        return false;

    SAVE_CVAR(sar_hud_default_spacing, Int);
    SAVE_CVAR(sar_hud_default_padding_x, Int);
    SAVE_CVAR(sar_hud_default_padding_y, Int);
    SAVE_CVAR(sar_hud_default_font_index, Int);
    SAVE_CVAR(sar_hud_default_font_color, String);
    SAVE_CVAR(sar_ihud_x, Int);
    SAVE_CVAR(sar_ihud_y, Int);
    SAVE_CVAR(sar_ihud_button_padding, Int);
    SAVE_CVAR(sar_ihud_button_size, Int);
    SAVE_CVAR(sar_ihud_button_color, String);
    SAVE_CVAR(sar_ihud_font_color, String);
    SAVE_CVAR(sar_ihud_font_index, Int);
    SAVE_CVAR(sar_ihud_layout, String);
    SAVE_CVAR(sar_ihud_shadow_color, String);
    SAVE_CVAR(sar_ihud_shadow_font_color, String);
    SAVE_CVAR(sar_sr_hud_x, Int);
    SAVE_CVAR(sar_sr_hud_y, Int);
    SAVE_CVAR(sar_sr_hud_font_color, String);
    SAVE_CVAR(sar_sr_hud_font_index, Int);

    file.close();
    return true;
}
bool Config::Load()
{
    if (Engine::GetGameDirectory == nullptr)
        return false;

    std::ifstream file(std::string(Engine::GetGameDirectory()) + this->filePath, std::ios::in);
    if (!file.good())
        return false;

    Engine::ExecuteCommand("exec _sar_cvars.cfg");

    file.close();
    return true;
}

Config* config;
extern Config* config;
