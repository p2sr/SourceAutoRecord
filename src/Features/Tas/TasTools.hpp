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
    int want_to_strafe;
    int strafing_direction;
    int is_vectorial;
    int strafe_type;

private:
    Vector acceleration;
    Vector prevVelocity;
    int prevTick;

public:
    TasTools();
    void AimAtPoint(float x, float y, float z);
    Vector GetVelocityAngles();
    Vector GetAcceleration();
    void* GetPlayerInfo();
    float GetStrafeAngle(CMoveData* pmove, int direction);
    void Strafe(CMoveData *pmove);
    void VectorialStrafe(CMoveData* pmove);
};

extern TasTools* tasTools;
