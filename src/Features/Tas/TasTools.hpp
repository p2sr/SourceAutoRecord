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
    int wantToStrafe;
    int strafingDirection;
    int isVectorial;
    int strafeType;
    int isTurning;

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
    float GetStrafeAngle(CMoveData* pMove, int direction);
    void Strafe(CMoveData *pMove);
    void VectorialStrafe(CMoveData* pMove);
};

extern TasTools* tasTools;
