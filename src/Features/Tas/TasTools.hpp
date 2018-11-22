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
    int move;

private:
    Vector acceleration;
    Vector prevVelocity;
    int prevTick;
    int m_first_tick_forward;
    int m_first_tick_backward;
    int m_first_tick_moveright;
    int m_first_tick_moveleft;

public:
    TasTools();
    void AimAtPoint(float x, float y, float z);
    Vector GetVelocityAngles();
    Vector GetAcceleration();
    void* GetPlayerInfo();
    void Strafe(int opposite, int in_2D);
};

extern TasTools* tasTools;
