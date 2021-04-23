#pragma once
#include <iostream>

#include "../TasPlayer.hpp"
#include "../TasTool.hpp"

/*
  AUTO STRAFING TOOL

*/


enum AutoStrafeType {
    DISABLED,
    VECTORIAL,
    ANGULAR
};

enum AutoStrafeParamType {
    SPECIFIED,
    CURRENT
};

struct AutoStrafeDirection {
    AutoStrafeParamType type;
    float angle;
};

struct AutoStrafeSpeed {
    AutoStrafeParamType type;
    float speed;
};


struct AutoStrafeParams : public TasToolParams {
    AutoStrafeType strafeType = DISABLED;
    AutoStrafeDirection strafeDir = { CURRENT, 0.0f };
    AutoStrafeSpeed strafeSpeed = { SPECIFIED, 10000.0f };
    bool turningPriority = false;

    AutoStrafeParams()
        : TasToolParams()
    {}

    AutoStrafeParams(AutoStrafeType type, AutoStrafeDirection dir, AutoStrafeSpeed speed, bool turningPriority)
        : TasToolParams(true)
        , strafeType(type)
        , strafeDir(dir)
        , strafeSpeed(speed)
        , turningPriority(turningPriority)
    {
    }
};

class AutoStrafeTool : public TasTool {
public:
    AutoStrafeTool(const char* name)
        : TasTool(name){};
    virtual AutoStrafeTool* GetTool();
    virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
    virtual void Apply(TasFramebulk& fb, const TasPlayerInfo& pInfo);
    virtual void Reset();

    Vector followLine;
    bool reachedAngle = false;
    bool reachedVelocity = false;

    Vector CreateWishDir(const TasPlayerInfo& player, float forwardMove, float sideMove);
    Vector GetGroundFrictionVelocity(const TasPlayerInfo& player);
    float GetMaxSpeed(const TasPlayerInfo& player, Vector wishDir, bool notAired = false);
    float GetMaxAccel(const TasPlayerInfo& player, Vector wishDir);

    Vector GetVelocityAfterMove(const TasPlayerInfo& player, float forwardMove, float sideMove);
    float GetFastestStrafeAngle(const TasPlayerInfo& player);
    float GetMaintainStrafeAngle(const TasPlayerInfo& player);

    float GetStrafeAngle(const TasPlayerInfo& player, AutoStrafeParams &dir);

    void MarkAngleReached(const TasPlayerInfo& pInfo);
};

extern AutoStrafeTool autoStrafeTool;

