#pragma once
#include "../TasPlayer.hpp"
#include "../TasTool.hpp"

#include <iostream>

/*
  AUTO STRAFING TOOL
*/


enum AutoStrafeType {
	DISABLED,
	VECTORIAL,
	ANGULAR,
	VECTORIAL_CAM
};

enum AutoStrafeParamType {
	SPECIFIED,
	CURRENT
};

struct AutoStrafeDirection {
	AutoStrafeParamType type;
	bool useVelAngle;
	float angle;
};

struct AutoStrafeSpeed {
	AutoStrafeParamType type;
	float speed;
};


struct AutoStrafeParams : public TasToolParams {
	AutoStrafeType strafeType = DISABLED;
	AutoStrafeDirection strafeDir = {CURRENT, true, 0.0f};
	AutoStrafeSpeed strafeSpeed = {SPECIFIED, 10000.0f};

	AutoStrafeParams()
		: TasToolParams() {}

	AutoStrafeParams(AutoStrafeType type, AutoStrafeDirection dir, AutoStrafeSpeed speed)
		: TasToolParams(true)
		, strafeType(type)
		, strafeDir(dir)
		, strafeSpeed(speed) {
	}
};


class AutoStrafeTool : public TasTool {
public:
	AutoStrafeTool(const char *name)
		: TasTool(name){};
	virtual AutoStrafeTool *GetTool();
	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo);
	virtual void Reset();

	Vector followLinePoint;
	bool shouldFollowLine = false;
	int lastTurnDir = 0;

	Vector GetGroundFrictionVelocity(const TasPlayerInfo &player);
	float GetMaxSpeed(const TasPlayerInfo &player, Vector wishDir, bool notAired = false);
	float GetMaxAccel(const TasPlayerInfo &player, Vector wishDir);
	Vector CreateWishDir(const TasPlayerInfo &player, float forwardMove, float sideMove);

	Vector GetVelocityAfterMove(const TasPlayerInfo &player, float forwardMove, float sideMove);
	float GetFastestStrafeAngle(const TasPlayerInfo &player);
	float GetTargetStrafeAngle(const TasPlayerInfo &player, float targetSpeed);
	float GetTurningStrafeAngle(const TasPlayerInfo &player);

	float GetStrafeAngle(const TasPlayerInfo &player, AutoStrafeParams &dir);

	int GetTurningDirection(const TasPlayerInfo &pInfo, float desAngle);
	void FollowLine(const TasPlayerInfo &pInfo);
};

extern AutoStrafeTool autoStrafeTool;
