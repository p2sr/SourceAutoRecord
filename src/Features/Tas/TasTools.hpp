#pragma once
#include <vector>

#include "Features/Feature.hpp"

#include "Utils/SDK.hpp"

#include "Command.hpp"

enum class PropType {
    Integer,
    Boolean,
    Float,
    Handle,
    Vector,
    String,
    Char
};

struct SetAnglesData {
    QAngle currentAngles = { 0, 0, 0 };
    QAngle targetAngles = { 0, 0, 0 };
    float speedInterpolation = 0;
};

class TasTools : public Feature {
public:
    char className[32];
    char propName[32];
    int propOffset;
    PropType propType;
    std::vector<SetAnglesData*> data;

private:
    Vector acceleration;
    Vector prevVelocity;
    int prevTick;

public:
    TasTools();
    ~TasTools();

    void AimAtPoint(void* player, float x, float y, float z, int doSlerp);
    Vector GetVelocityAngles(void* player);
    Vector GetAcceleration(void* player);
    void* GetPlayerInfo();
    void SetAngles(void* pPlayer);
    QAngle Slerp(QAngle a0, QAngle a1, float speedInterpolation);
};

extern TasTools* tasTools;

extern Command sar_tas_aim_at_point;
extern Command sar_tas_set_prop;
extern Command sar_tas_addang;
extern Command sar_tas_setang;
