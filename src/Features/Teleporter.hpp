#pragma once
#include "Feature.hpp"

#include "Utils/SDK.hpp"

class Teleporter : public Feature {
private:
    bool isSet;
    Vector origin;
    QAngle angles;

public:
    Teleporter();
    void Save();
    void Teleport();
    bool HasLocation();
    void Reset();
};

extern Teleporter* teleporter;
