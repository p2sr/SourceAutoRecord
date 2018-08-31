#pragma once
#include "Feature.hpp"

#include "Utils/SDK.hpp"

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

extern Teleporter* teleporter;
