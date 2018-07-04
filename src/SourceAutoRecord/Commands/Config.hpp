#pragma once
#include "Modules/Console.hpp"

#include "Features/Config.hpp"

#include "Command.hpp"

CON_COMMAND(sar_cvars_save, "Saves important SAR cvars.\n")
{
    if (!Config::Save()) {
        Console::Print("Failed to create config file!\n");
    } else {
        Console::Print("Saved important settings in /cfg/_sar_cvars.cfg!\n");
    }
}

CON_COMMAND(sar_cvars_load, "Loads important SAR cvars.\n")
{
    if (!Config::Load()) {
        Console::Print("Config file not found!\n");
    }
}