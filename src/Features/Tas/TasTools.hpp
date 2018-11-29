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
    int m_forward_was_pressed;
    int m_backward_was_pressed;
    int m_moveright_was_pressed;
    int m_moveleft_was_pressed;
    int m_jump_was_pressed;

public:
    TasTools();
    void AimAtPoint(float x, float y, float z);
    Vector GetVelocityAngles();
    Vector GetAcceleration();
    void* GetPlayerInfo();
    void Strafe(int opposite);
    void SetButtonBits(int buttonBits);
    void SetMoveButtonsState(float forward, float backward, float moveright, float moveleft, float jump);
};

extern TasTools* tasTools;
