#pragma once
#include "Modules/Console.hpp"

#include "Features/Teleporter.hpp"

#include "Cheats.hpp"
#include "Command.hpp"

CON_COMMAND(sar_teleport, "Teleports the player to the last saved location.\n")
{
    if (sv_cheats.GetBool()) {
        if (teleporter->isSet) {
            teleporter->Teleport();
        } else {
            console->Print("Location not set. Use sar_teleport_setpos.\n");
        }
    } else {
        console->Print("Cannot teleport without sv_cheats 1.\n");
    }
}

CON_COMMAND(sar_teleport_setpos, "Saves current location for teleportation.\n")
{
    teleporter->Save();
}
