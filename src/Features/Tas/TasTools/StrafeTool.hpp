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
	CURRENT,
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
	AutoStrafeSpeed strafeSpeed = {CURRENT};
	bool noPitchLock = false;
	bool antiSpeedLock = true;
	float turnRate = 1.0f;
	float force = 1.0f;

	AutoStrafeParams()
		: TasToolParams() {}

	AutoStrafeParams(AutoStrafeType type, AutoStrafeDirection dir, AutoStrafeSpeed speed, bool noPitchLock, bool antiSpeedLock, float turnRate, float force)
		: TasToolParams(true)
		, strafeType(type)
		, strafeDir(dir)
		, strafeSpeed(speed)
		, noPitchLock(noPitchLock)
		, antiSpeedLock(antiSpeedLock)
		, turnRate(turnRate)
		, force(force) {
	}
};


class AutoStrafeTool : public TasToolWithParams<AutoStrafeParams> {
private:
	Vector followLinePoint;
	bool shouldFollowLine = false;
	bool switchedFromVeccam = false;
	int lastTurnDir = 0;
public:
	AutoStrafeTool(int slot)
		: TasToolWithParams("strafe", POST_PROCESSING, slot) {};
	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo);

	Vector GetGroundFrictionVelocity(const TasPlayerInfo &player);
	float GetMaxSpeed(const TasPlayerInfo &player, Vector wishDir, bool notAired = false);
	float GetMaxAccel(const TasPlayerInfo &player, Vector wishDir);
	Vector CreateWishDir(const TasPlayerInfo &player, float forwardMove, float sideMove);
	Vector GetAirLockFactorVector(const TasPlayerInfo &player);

	Vector GetVelocityAfterMove(const TasPlayerInfo &player, float forwardMove, float sideMove);
	float GetFastestStrafeAngle(const TasPlayerInfo &player);
	float GetTargetStrafeAngle(const TasPlayerInfo &player, float targetSpeed, float turningDir);
	float GetTurningStrafeAngle(const TasPlayerInfo &player);

private:
	void UpdateTargetValuesMarkedCurrent(TasFramebulk &fb, const TasPlayerInfo &pInfo);
	bool IsMovementAffectedByPitch(const TasPlayerInfo &pInfo);
	bool TryReachTargetValues(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
	void ApplyStrafe(TasFramebulk &bulk, const TasPlayerInfo &pInfo);

	float GetStrafeAngle(const TasPlayerInfo &player, AutoStrafeParams &dir);

	int GetTurningDirection(const TasPlayerInfo &pInfo, float desAngle);
	void FollowLine(const TasPlayerInfo &pInfo);
};

extern AutoStrafeTool autoStrafeTool[2];
