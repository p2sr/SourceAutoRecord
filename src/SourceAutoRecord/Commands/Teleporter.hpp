#pragma once
#include "Modules/Console.hpp"

#include "Features/Teleporter.hpp"

#include "Cheats.hpp"
#include "Command.hpp"

CON_COMMAND(sar_teleport, "Teleports the player to the last saved location.\n")
{
    if (Cheats::sv_cheats.GetBool()) {
        if (Teleporter::IsSet) {
            Teleporter::Teleport();
        } else {
            Console::Print("Location not set. Use sar_teleport_setpos.\n");
        }
    } else {
        Console::Print("Cannot teleport without sv_cheats 1.\n");
    }
}

CON_COMMAND(sar_teleport_setpos, "Saves current location for teleportation.\n")
{
    Teleporter::Save();
}