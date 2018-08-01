#pragma once
#include "Modules/Console.hpp"

#include "Features/Config.hpp"

#include "Command.hpp"

CON_COMMAND(sar_cvars_save, "Saves important SAR cvars.\n")
{
    if (!Config::Save()) {
        console->Print("Failed to create config file!\n");
    } else {
        console->Print("Saved important settings in /cfg/_sar_cvars.cfg!\n");
    }
}

CON_COMMAND(sar_cvars_load, "Loads important SAR cvars.\n")
{
    if (!Config::Load()) {
        console->Print("Config file not found!\n");
    }
}