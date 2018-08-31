#pragma once
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"

#include "Features/Feature.hpp"

#include "Utils.hpp"

class Teleporter : public Feature {
public:
    bool isSet;
    Vector origin;
    QAngle angles;

public:
    Teleporter();
    void Save();
    void Teleport();
};

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
    this->origin = Client::GetAbsOrigin();
    this->angles = Engine::GetAngles();
    console->Print("Saved location: %.3f %.3f %.3f\n", this->origin.x, this->origin.y, this->origin.z);
}
void Teleporter::Teleport()
{
    Engine::SetAngles(this->angles);
    char setpos[64];
    snprintf(setpos, sizeof(setpos), "setpos %f %f %f", this->origin.x, this->origin.y, this->origin.z);
    Engine::ExecuteCommand(setpos);
}

Teleporter* teleporter;
extern Teleporter* teleporter;
