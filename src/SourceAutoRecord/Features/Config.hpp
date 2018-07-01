#pragma once
#include "Modules/Engine.hpp"

#include "Utils.hpp"

namespace Config {

std::string FilePath("/cfg/_sar_cvars.cfg");

bool Save()
{
    if (Engine::GetGameDirectory == nullptr)
        return false;

    std::ofstream file(std::string(Engine::GetGameDirectory()) + FilePath, std::ios::out | std::ios::trunc);
    if (!file.good())
        return false;

    auto spacing = sar_hud_default_spacing.GetInt();
    auto xpadding = sar_hud_default_padding_x.GetInt();
    auto ypadding = sar_hud_default_padding_y.GetInt();
    auto index = sar_hud_default_font_index.GetInt();
    auto color = sar_hud_default_font_color.GetString();

    file << "sar_hud_default_spacing " << spacing << "\n";
    file << "sar_hud_default_padding_x " << xpadding << "\n";
    file << "sar_hud_default_padding_y " << ypadding << "\n";
    file << "sar_hud_default_font_index " << index << "\n";
    file << "sar_hud_default_font_color " << color;

    file.close();
    return true;
}
bool Load()
{
    if (Engine::GetGameDirectory == nullptr)
        return false;

    std::ifstream file(std::string(Engine::GetGameDirectory()) + FilePath, std::ios::in);
    if (!file.good())
        return false;

    Engine::ExecuteCommand("exec _sar_cvars.cfg");

    file.close();
    return true;
}
}