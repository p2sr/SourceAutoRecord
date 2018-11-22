#include "Teleporter.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Command.hpp"
#include "Utils.hpp"

Teleporter* teleporter;

Teleporter::Teleporter()
    : isSet(false)
    , origin()
    , angles()
{
    this->hasLoaded = true;
}
void Teleporter::Save()
{
    this->origin = client->GetAbsOrigin();
    this->angles = engine->GetAngles();
    this->isSet = true;

    console->Print("Saved location: %.3f %.3f %.3f\n", this->origin.x, this->origin.y, this->origin.z);
}
void Teleporter::Teleport()
{
    char setpos[64];
    std::snprintf(setpos, sizeof(setpos), "setpos %f %f %f", this->origin.x, this->origin.y, this->origin.z);

    engine->SetAngles(this->angles);
    engine->ExecuteCommand(setpos);
}
bool Teleporter::HasLocation()
{
    return this->isSet;
}
void Teleporter::Reset()
{
    this->isSet = false;
}

CON_COMMAND(sar_teleport, "Teleports the player to the last saved location.\n")
{
    if (!sv_cheats.GetBool()) {
        return console->Print("Cannot teleport without sv_cheats 1.\n");
    }

    if (!teleporter->HasLocation()) {
        return console->Print("Location not set. Use sar_teleport_setpos.\n");
    }

    teleporter->Teleport();
}
CON_COMMAND(sar_teleport_setpos, "Saves current location for teleportation.\n")
{
    teleporter->Save();
}
