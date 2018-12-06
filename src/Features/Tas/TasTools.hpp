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

struct MoveInputs {
    float forward;
    float backward;
    float moveleft;
    float moveright;
    float jump;
};

class TasTools : public Feature {
public:
    char className[32];
    char propName[32];
    int propOffset;
    PropType propType;
    int want_to_strafe;
    int strafing_direction;
    int strafeMode;

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
    MoveInputs moves;

public:
    TasTools();
    void AimAtPoint(float x, float y, float z);
    Vector GetVelocityAngles();
    Vector GetAcceleration();
    void* GetPlayerInfo();
    float GetStrafeAngle(CMoveData* pmove, int direction);
    void Strafe(CMoveData *pmove);
    void VectorialStrafe(CMoveData* pmove);
    void SetButtonBits(int buttonBits);
    void SetMoveButtonsState(float forward, float backward, float moveright, float moveleft, float jump);
    MoveInputs GetMoveInputs();
};

extern TasTools* tasTools;
