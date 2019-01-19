#include "Teleporter.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Command.hpp"
#include "Utils.hpp"

Teleporter* teleporter;

Teleporter::Teleporter()
    : locations()
{
    this->hasLoaded = true;
}
TeleportLocation* Teleporter::GetLocation(int nSlot)
{
    return &this->locations[nSlot];
}
void Teleporter::Save(int nSlot)
{
    auto player = server->GetPlayer(nSlot);
    if (player) {
        auto location = this->GetLocation(nSlot);
        location->origin = server->GetAbsOrigin(player);
        location->angles = server->GetAbsAngles(player);
        location->isSet = true;

        console->Print("Saved location: %.3f %.3f %.3f\n", location->origin.x, location->origin.y, location->origin.z);
    }
}
void Teleporter::Teleport(int nSlot)
{
    auto location = this->GetLocation(nSlot);

    char setpos[64];
    std::snprintf(setpos, sizeof(setpos), "setpos %f %f %f", location->origin.x, location->origin.y, location->origin.z);

    engine->SetAngles(location->angles);
    engine->ExecuteCommand(setpos);
}

CON_COMMAND(sar_teleport, "Teleports the player to the last saved location.\n")
{
    if (!sv_cheats.GetBool()) {
        return console->Print("Cannot teleport without sv_cheats 1!\n");
    }

    auto slot = GET_SLOT();
    if (!teleporter->GetLocation(slot)->isSet) {
        return console->Print("Location not set! Use sar_teleport_setpos.\n");
    }

    teleporter->Teleport(slot);
}
CON_COMMAND(sar_teleport_setpos, "Saves current location for teleportation.\n")
{
    teleporter->Save(GET_SLOT());
}
