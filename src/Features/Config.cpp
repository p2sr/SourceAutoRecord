#include "Config.hpp"

#include <fstream>
#include <string>

#include "Hud/Hud.hpp"
#include "Hud/InputHud.hpp"
#include "Hud/InspectionHud.hpp"
#include "Hud/SpeedrunHud.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Game.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

#define SAVE_CVAR(cvar, value) \
    file << #cvar " \"" << cvar.Get##value() << "\"\n";

Config* config;

Config::Config()
    : filePath("/cfg/_sar_cvars.cfg")
{
    this->hasLoaded = true;
}
bool Config::Save()
{
    std::ofstream file(std::string(engine->GetGameDirectory()) + this->filePath, std::ios::out | std::ios::trunc);
    if (!file.good()) {
        file.close();
        return false;
    }

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
    SAVE_CVAR(sar_ei_hud_x, Int);
    SAVE_CVAR(sar_ei_hud_y, Int);
    SAVE_CVAR(sar_ei_hud_z, Int);
    SAVE_CVAR(sar_ei_hud_font_color, String);
    SAVE_CVAR(sar_ei_hud_font_color2, String);
    SAVE_CVAR(sar_ei_hud_font_index, Int);

    if (sar.game->Is(SourceGame_Portal2Game | SourceGame_Portal)) {
        SAVE_CVAR(sar_sr_hud_x, Int);
        SAVE_CVAR(sar_sr_hud_y, Int);
        SAVE_CVAR(sar_sr_hud_font_color, String);
        SAVE_CVAR(sar_sr_hud_font_index, Int);
    }

    file.close();
    return true;
}
bool Config::Load()
{
    std::ifstream file(std::string(engine->GetGameDirectory()) + this->filePath, std::ios::in);
    if (!file.good()) {
        file.close();
        return false;
    }

    engine->ExecuteCommand("exec _sar_cvars.cfg");

    file.close();
    return false;
}
