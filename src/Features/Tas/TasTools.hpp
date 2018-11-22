#pragma once
#include "Features/Feature.hpp"
#include "Utils/SDK.hpp"

class TasTools : public Feature {

public:
    TasTools();
    void AimAtPoint(float x, float y, float z);
    Vector GetVelocityAngles();
    Vector GetAcceleration();
    int GetOffset();
    void Strafe(int opposite, int grounded, int in_2D);

public:
    char m_offset_name[100];
    int m_offset;
    void* m_player;
    int move;

private:
    Vector m_previous_speed;
    int m_last_tick;
    int m_first_tick_forward;
    int m_first_tick_backward;
    int m_first_tick_moveright;
    int m_first_tick_moveleft;
    Vector m_acceleration;
};
extern TasTools* tasTools;
