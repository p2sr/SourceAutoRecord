#pragma once
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Teleporter.hpp"

namespace Callbacks {

void Teleport()
{
    if (sv_cheats.GetBool()) {
        if (Teleporter::IsSet) {
            Teleporter::Teleport();
        } else {
            Console::Print("Location not set. Use sar_teleport_setpos.\n");
        }
    } else {
        Console::Print("Cannot teleport without sv_cheats 1.\n");
    }
}
void SetTeleport()
{
    Teleporter::Save();
}
}