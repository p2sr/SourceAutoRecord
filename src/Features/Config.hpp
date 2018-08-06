#pragma once
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Cheats.hpp"
#include "Command.hpp"
#include "Utils.hpp"

#define SAVE_CVAR(cvar, value) \
    file << #cvar " " << cvar.Get##value() << "\n";

class Config {
public:
    std::string filePath;

public:
    Config();
    bool Save();
    bool Load();
};

Config* config;
extern Config* config;

Config::Config()
    : filePath("/cfg/_sar_cvars.cfg")
{
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

CON_COMMAND(sar_cvars_save, "Saves important SAR cvars.\n")
{
    if (!config->Save()) {
        console->Print("Failed to create config file!\n");
    } else {
        console->Print("Saved important settings in /cfg/_sar_cvars.cfg!\n");
    }
}

CON_COMMAND(sar_cvars_load, "Loads important SAR cvars.\n")
{
    if (!config->Load()) {
        console->Print("Config file not found!\n");
    }
}
