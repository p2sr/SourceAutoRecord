#pragma once
#include "Features/Feature.hpp"
#include "Utils/SDK.hpp"

class TasTools : public Feature {

public:
    TasTools();
    void AimAtPoint(float x, float y, float z);
    Vector GetVelocityAngles();
    Vector& GetAcceleration();
    int GetOffset();


public:
    char m_class_name[100];
    char m_offset_name[100];

private:
    Vector m_previous_speed;
    int m_last_tick;
    Vector m_acceleration;
};

extern TasTools* tasTools;
