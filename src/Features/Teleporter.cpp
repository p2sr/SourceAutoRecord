#include "Teleporter.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Feature.hpp"

#include "Utils.hpp"

Teleporter::Teleporter()
    : isSet(false)
    , origin()
    , angles()
{
    this->hasLoaded = true;
}
void Teleporter::Save()
{
    this->isSet = true;
    this->origin = client->GetAbsOrigin();
    this->angles = engine->GetAngles();
    console->Print("Saved location: %.3f %.3f %.3f\n", this->origin.x, this->origin.y, this->origin.z);
}
void Teleporter::Teleport()
{
    engine->SetAngles(this->angles);
    char setpos[64];
    snprintf(setpos, sizeof(setpos), "setpos %f %f %f", this->origin.x, this->origin.y, this->origin.z);
    engine->ExecuteCommand(setpos);
}

Teleporter* teleporter;
