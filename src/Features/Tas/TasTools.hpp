#pragma once
#include "Features/Feature.hpp"

#include "Utils/SDK.hpp"

enum class PropType {
    Integer,
    Boolean,
    Float,
    Handle,
    Vector,
    String,
    Char
};

class TasTools : public Feature {
public:
    char className[32];
    char propName[32];
    int propOffset;
    PropType propType;

private:
    Vector acceleration;
    Vector prevVelocity;
    int prevTick;
    int buttonBits;

public:
    TasTools();
    void AimAtPoint(float x, float y, float z);
    Vector GetVelocityAngles();
    Vector GetAcceleration();
    void* GetPlayerInfo();
    void Strafe(int opposite, int in_2D);
    void SetButtonBits(int buttonBits);
};

extern TasTools* tasTools;
