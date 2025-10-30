#include "StrafeTool.hpp"

#include "../TasParser.hpp"
#include "JumpTool.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "TasUtils.hpp"
#include "Utils/SDK.hpp"

#include <cfloat>

AutoStrafeTool autoStrafeTool[2] = {{0}, {1}};

void AutoStrafeTool::Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo) {
	if (!params.enabled) {
		return;
	}

	//create fake player info for a sake of values being correct
	TasPlayerInfo fakePlayerInfo = pInfo;

	FOR_TAS_SCRIPT_VERSIONS_UNTIL(7) {
		// handled by TasPlayer in newer versions
		if (autoJumpTool[this->slot].GetCurrentParams().enabled || jumpTool[this->slot].GetCurrentParams().enabled) {
			// if autojump is enabled, we're never grounded.
			fakePlayerInfo.willBeGrounded = false;
		}
	}

	// adjusting fake pinfo to have proper angles (after rotation)
	fakePlayerInfo.angles.y -= fb.viewAnalog.x;
	fakePlayerInfo.angles.x -= fb.viewAnalog.y;

	// when not grounded, air acceleration is not optimal if pitch is not
	// in a range between -30 and 30 degrees (both exclusive). making sure
	// it's in the right range, unless specifically asked to not do it.
	if (IsMovementAffectedByPitch(fakePlayerInfo) && !params.noPitchLock) {
		float signAng = fakePlayerInfo.angles.x;
		FOR_TAS_SCRIPT_VERSIONS_UNTIL(5) {
			signAng = pInfo.angles.x;
		}
		float desiredAngle = (29.9999f * (signAng / absOld(signAng)));
		fb.viewAnalog.y = pInfo.angles.x - desiredAngle;
		fakePlayerInfo.angles.x = desiredAngle;
	}

	// update parameters that has type CURRENT
	if (this->updated) {
		this->shouldFollowLine = false;
		this->lastTurnDir = 0;
		this->switchedFromVeccam = false;

		UpdateTargetValuesMarkedCurrent(fb, pInfo); // using real angles instead of fake ones here.

		this->updated = false;
	}

	bool reachedTargetValues = false;
	FOR_TAS_SCRIPT_VERSIONS_SINCE(8) {
		reachedTargetValues = TryReachTargetValues(fb, fakePlayerInfo);
	}
	if (!reachedTargetValues) {
		ApplyStrafe(fb, fakePlayerInfo);
	}

	//pitch lock isn't used. try to compensate the pitch movement multiplication by dividing it now
	if (IsMovementAffectedByPitch(fakePlayerInfo) && params.noPitchLock) {
		fb.moveAnalog.y /= cosOld(DEG2RAD(fakePlayerInfo.angles.x));
	}

}

void AutoStrafeTool::UpdateTargetValuesMarkedCurrent(TasFramebulk &fb, const TasPlayerInfo &pInfo) {
	if (params.strafeDir.type == CURRENT) {
		if (params.strafeDir.useVelAngle) {
			float velAngle = TasUtils::GetVelocityAngles(&pInfo).x;
			FOR_TAS_SCRIPT_VERSIONS_SINCE(6) {
				if (pInfo.velocity.Length2D() == 0) {
					velAngle = pInfo.angles.y;
				}
			}

			params.strafeDir.angle = velAngle;
			FollowLine(pInfo);
		} else {
			params.strafeDir.angle = pInfo.angles.y;
		}
	}

	if (params.strafeSpeed.type == CURRENT) {
		params.strafeSpeed.speed = pInfo.velocity.Length2D();
	}
}

bool AutoStrafeTool::IsMovementAffectedByPitch(const TasPlayerInfo &pInfo) {
	return !pInfo.willBeGrounded && fabsf(pInfo.angles.x) >= 30.0f;
}

bool AutoStrafeTool::TryReachTargetValues(TasFramebulk &bulk, const TasPlayerInfo &pInfo) {
	// Attempting to check if target velocity and angle can be reached within a single tick,
	// in which case, we'll try to be precise about the force of our movement input.

	// our target angle is too big to be ever reached
	if (fabsf(params.strafeDir.angle) > 360.0f) {
        return false;
    }

	Vector velocity = GetGroundFrictionVelocity(pInfo);
	velocity.z = 0;

	float targetAngleRad = DEG2RAD(params.strafeDir.angle);
	Vector targetVel = Vector{cosf(targetAngleRad), sinf(targetAngleRad), 0} * params.strafeSpeed.speed;
	Vector targetVelDelta = targetVel - velocity;

	if (targetVelDelta == Vector(0, 0, 0)) {
		bulk.moveAnalog.x = bulk.moveAnalog.y = 0;
        return true;
    }

	// clamp velocity delta to air control limit, as we're unable to decelerate
	Vector airLockFactorVector = GetAirLockFactorVector(pInfo);
    if (airLockFactorVector.x * targetVelDelta.x < 0) {
        targetVelDelta.x = 0;
    }
    if (airLockFactorVector.y * targetVelDelta.y < 0) {
        targetVelDelta.y = 0;
    }

	Vector absoluteWishDir = targetVelDelta.Normalize();
	float absoluteWishDirAngleRad = atan2f(absoluteWishDir.y, absoluteWishDir.x);

	float playerYawRad = DEG2RAD(pInfo.angles.y);
	float wishDirAngleRad = absoluteWishDirAngleRad - playerYawRad;
	float forwardMove = cosf(wishDirAngleRad) * params.force;
    float sideMove = -sinf(wishDirAngleRad) * params.force;

	Vector velocityAfterMove = GetVelocityAfterMove(pInfo, forwardMove, sideMove);
	Vector afterMoveDelta = velocityAfterMove - velocity;

	float afterMoveDeltaLength = afterMoveDelta.Length2D();
	float targetVelDeltaLength = targetVelDelta.Length2D();

	if (afterMoveDeltaLength >= targetVelDeltaLength) {
		if (params.strafeType == AutoStrafeType::VECTORIAL_CAM) {
			float angleDelta = params.strafeDir.angle - pInfo.angles.y;
			bulk.viewAnalog.x -= angleDelta;

			wishDirAngleRad -= DEG2RAD(angleDelta);

			forwardMove = cosf(wishDirAngleRad);
			sideMove = -sinf(wishDirAngleRad);
		}

		float inputScale = targetVelDeltaLength / afterMoveDeltaLength;
		bulk.moveAnalog.x = sideMove * inputScale;
		bulk.moveAnalog.y = forwardMove * inputScale;

        return true;
    }

	return false;
}

void AutoStrafeTool::ApplyStrafe(TasFramebulk &fb, const TasPlayerInfo &pInfo) {
	float velAngle = TasUtils::GetVelocityAngles(&pInfo).x;

	FOR_TAS_SCRIPT_VERSIONS_SINCE(6) {
		if (pInfo.velocity.Length2D() == 0) {
			velAngle = params.strafeDir.angle;
		}
	}

	float angle = velAngle + RAD2DEG(this->GetStrafeAngle(pInfo, params)) * params.turnRate;
	
	FOR_TAS_SCRIPT_VERSIONS_SINCE(8) {
		// Deal with airlock: we're strafing, so always try to maximize how much
		// we can turn while also "trying" to meet speed criteria.
		Vector absoluteMoveDirection{cosf(DEG2RAD(angle)), sinf(DEG2RAD(angle)), 0};

		Vector airLockFactor = GetAirLockFactorVector(pInfo);
		bool isDecelerating = pInfo.velocity.Length2D() > params.strafeSpeed.speed;

		if (airLockFactor.x != 0) {
			if (airLockFactor.x * absoluteMoveDirection.x < 0 || isDecelerating) {
				absoluteMoveDirection.x = 0;
			}
		}
		if (airLockFactor.y != 0) {
			if (airLockFactor.y * absoluteMoveDirection.y < 0 || isDecelerating) {
				absoluteMoveDirection.y = 0;
			}
		}

		angle = RAD2DEG(atan2f(absoluteMoveDirection.y, absoluteMoveDirection.x));
	}

	// applying the calculated angle depending on the type of strafing
	if (params.strafeType == AutoStrafeType::VECTORIAL) {
		float moveAngle = DEG2RAD(angle - pInfo.angles.y);
		fb.moveAnalog.x = -sinf(moveAngle);
		fb.moveAnalog.y = cosf(moveAngle);
		if (pInfo.onSpeedPaint) fb.moveAnalog.x *= 2;
	} else if (params.strafeType == AutoStrafeType::VECTORIAL_CAM) {
		float lookAngle = this->shouldFollowLine ? params.strafeDir.angle : velAngle;
		fb.viewAnalog.x -= lookAngle - pInfo.angles.y;
		FOR_TAS_SCRIPT_VERSIONS_UNTIL(5) {
			fb.viewAnalog.x = -(lookAngle - pInfo.angles.y);
		}
		float moveAngle = DEG2RAD(angle - lookAngle);
		fb.moveAnalog.x = -sinf(moveAngle);
		fb.moveAnalog.y = cosf(moveAngle);
		if (pInfo.onSpeedPaint) fb.moveAnalog.x *= 2;
	} else if (params.strafeType == AutoStrafeType::ANGULAR) {
		//	making sure moveAnalog is always at maximum value.
		if (fb.moveAnalog.Length2D() == 0) {
			fb.moveAnalog.y = 1;
		} else {
			fb.moveAnalog = fb.moveAnalog.Normalize();
		}

		float lookAngle = RAD2DEG(atan2f(fb.moveAnalog.x, fb.moveAnalog.y));

		QAngle newAngle = {0, angle + lookAngle, 0};
		fb.viewAnalog.x -= newAngle.y - pInfo.angles.y;
	}

	fb.moveAnalog *= params.force;
}

// returns player's velocity after its been affected by ground friction
Vector AutoStrafeTool::GetGroundFrictionVelocity(const TasPlayerInfo &player) {
	// Getting player's friction
	// it is important to include a right value, as it is modified on a slowfly effect.
	float friction = sv_friction.GetFloat() * player.surfaceFriction;

	Vector vel = player.velocity;

	if (player.willBeGrounded) {
		if (vel.Length2D() >= sv_stopspeed.GetFloat()) {
			vel = vel * (1.0f - player.ticktime * friction);
		} else if (vel.Length2D() >= fmaxf(0.1f, player.ticktime * sv_stopspeed.GetFloat() * friction)) {
			// lambda -= v * tau * stop * friction
			vel = vel - (vel.Normalize() * (player.ticktime * sv_stopspeed.GetFloat() * friction));
		} else {
			vel = Vector();
		}

		if (vel.Length2D() < 1.0) {
			vel = Vector();
		}
	}

	return vel;
}

// returns max speed value which is used by autostrafer math
float AutoStrafeTool::GetMaxSpeed(const TasPlayerInfo &player, Vector wishDir, bool notAired) {
	// calculate max speed based on player inputs, grounded and ducking states.
	float duckMultiplier = (player.willBeGrounded && player.ducked) ? (1.0f / 3.0f) : 1.0f;
	float waterMultiplier = 1.0f;
	if (sar.game->Is(SourceGame_INFRA)) {
		duckMultiplier = 1.0f; // idk man. 1/2 seems correct but this produces better results.
		if (player.waterLevel == WL_Feet) {
			waterMultiplier = 0.75f;
		}
	}
	wishDir.y *= player.maxSpeed;
	wishDir.x *= player.maxSpeed;
	float maxSpeed = fminf(player.maxSpeed, wishDir.Length2D()) * duckMultiplier * waterMultiplier;
	float maxAiredSpeed = (player.willBeGrounded || notAired) ? maxSpeed : fminf(60, maxSpeed);

	return maxAiredSpeed;
}


float AutoStrafeTool::GetMaxAccel(const TasPlayerInfo &player, Vector wishDir) {
	bool aircon2 = sar_aircontrol.GetInt() == 2 && server->AllowsMovementChanges();
	float accel = player.willBeGrounded ? sv_accelerate.GetFloat() 
				: aircon2 ? sv_airaccelerate.GetFloat() 
				: sv_paintairacceleration.GetFloat();
	float realAccel = player.surfaceFriction * player.ticktime * GetMaxSpeed(player, wishDir, true) * accel;
	return realAccel;
}


Vector AutoStrafeTool::CreateWishDir(const TasPlayerInfo &player, float forwardMove, float sideMove) {
	Vector wishDir(sideMove, forwardMove);
	if (wishDir.Length() > 1.f) {
		wishDir = wishDir.Normalize();
	}

	if (sar_aircontrol.GetInt() != 2 || !server->AllowsMovementChanges()) {
		// forwardmove is affected by player pitch when in air
		// but only with pitch outside of range from -30 to 30 deg (both exclusive)
		if (!player.willBeGrounded) {
			if (absOld(player.angles.x) >= 30.0f) {
				wishDir.y *= cosOld(DEG2RAD(player.angles.x));
			}
		}
	}

	//rotating wishDir
	float yaw = DEG2RAD(player.angles.y);
	wishDir = Vector(sinOld(yaw) * wishDir.x + cosOld(yaw) * wishDir.y, -cosOld(yaw) * wishDir.x + sinOld(yaw) * wishDir.y);

	// air control limit
	Vector airLockFactorVector = GetAirLockFactorVector(player);
	if (airLockFactorVector.x * wishDir.x < 0) {
		wishDir.x = 0;
	}
	if (airLockFactorVector.y * wishDir.y < 0) {
		wishDir.y = 0;
	}

	return wishDir;
}

Vector AutoStrafeTool::GetAirLockFactorVector(const TasPlayerInfo &player) {
	Vector airLockFactorVector{0, 0, 0};

	float airConLimit = (sar_aircontrol.GetBool() && server->AllowsMovementChanges()) ? INFINITY : 300;

	if (!player.willBeGrounded && player.velocity.Length2D() > airConLimit) {
		float absVelX = absOld(player.velocity.x);
		if (absVelX > airConLimit * 0.5) {
			airLockFactorVector.x = player.velocity.x / absVelX;
		}
		float absVelY = absOld(player.velocity.y);
		if (absVelY > airConLimit * 0.5) {
			airLockFactorVector.y = player.velocity.y / absVelY;
		}
	}

	return airLockFactorVector;
}


// returns the predicted velocity in the next tick
Vector AutoStrafeTool::GetVelocityAfterMove(const TasPlayerInfo &player, float forwardMove, float sideMove) {
	Vector velocity = GetGroundFrictionVelocity(player);

	//create wishdir for calculations
	Vector wishDir = CreateWishDir(player, forwardMove, sideMove);

	//no movement means velocity is only affected by ground friction
	if (wishDir.Length2D() == 0) return velocity;

	// get max speed and acceleration
	float maxSpeed = GetMaxSpeed(player, wishDir);
	float maxAccel = GetMaxAccel(player, wishDir);

	// limiting the velocity
	float accelDiff = maxSpeed - velocity.Dot(wishDir.Normalize());

	if (accelDiff <= 0) return velocity;

	float accelForce = fminf(maxAccel, accelDiff);

	return velocity + wishDir.Normalize() * accelForce;
}

// get horizontal angle of wishdir that would give you the fastest acceleration
// angle is relative to your current velocity direction.
float AutoStrafeTool::GetFastestStrafeAngle(const TasPlayerInfo &player) {
	Vector velocity = GetGroundFrictionVelocity(player);

	if (velocity.Length2D() == 0) return 0;

	Vector wishDir = Vector(0, 1) * params.force;
	float maxSpeed = GetMaxSpeed(player, wishDir);
	float maxAccel = GetMaxAccel(player, wishDir);

	// finding the most optimal angle. 
	// formula shamelessly taken from https://www.jwchong.com/hl/movement.html
	float cosAng = (maxSpeed - maxAccel) / velocity.Length2D();

	return acosf(fminf(fmaxf(cosAng,0.0f),1.0f));
}

// get horizontal angle of wishdir that would achieve given velocity
// angle is relative to your current velocity direction.
float AutoStrafeTool::GetTargetStrafeAngle(const TasPlayerInfo &player, float targetSpeed, float turningDir) {
	Vector vel = GetGroundFrictionVelocity(player);

	float currentSpeed = vel.Length2D();
	if (currentSpeed == 0) return 0;

	Vector wishDir = Vector(0, 1) * params.force;
	float maxAccel = GetMaxAccel(player, wishDir);

	// Assuming that it is possible to achieve a velocity of a given length,
	// I'm using a law of cosines to get the right angle for acceleration.
	float targetAngle = acosf(-(powOld(currentSpeed, 2) + powOld(maxAccel, 2) - powOld(targetSpeed, 2)) / (2.0f * currentSpeed * maxAccel)) * turningDir;

	FOR_TAS_SCRIPT_VERSIONS_SINCE(8) {
		// When we're affected by pitch, we should recalculate the angle a couple of times to ensure we can reach it.
		if (params.noPitchLock && IsMovementAffectedByPitch(player)) {
			float velAngle = TasUtils::GetVelocityAngles(&player).x;
			float velToLocalAngleDeltaRad = DEG2RAD(player.angles.y) - DEG2RAD(velAngle);
			float cosPitch = cosf(DEG2RAD(player.angles.x));

			const int maxIterations = 50;
			for (int i = 0; i < maxIterations; i++) {
				float correctAng = targetAngle - velToLocalAngleDeltaRad;
				float forwardmove = cosf(correctAng) / cosPitch;
				float sidemove = -sinf(correctAng);

				float scaledAccel = maxAccel / sqrtf(forwardmove * forwardmove + sidemove * sidemove);
				float previousTargetAngle = targetAngle;
				targetAngle = acosf(-(powf(currentSpeed, 2) + powf(scaledAccel, 2) - powf(targetSpeed, 2)) / (2.0f * currentSpeed * scaledAccel)) * turningDir;

				if (fabsf(previousTargetAngle - targetAngle) < 0.00001f) {
					break;
				}
			}
		}
	}

	return targetAngle;
}

// get horizontal angle of wishdir that would give the biggest turning angle in given tick
// angle is relative to your current velocity direction.
float AutoStrafeTool::GetTurningStrafeAngle(const TasPlayerInfo &player) {
	Vector velocity = GetGroundFrictionVelocity(player);

	if (velocity.Length2D() == 0) return 0;

	Vector wishDir = Vector(0, 1) * params.force;
	float maxAccel = GetMaxAccel(player, wishDir);

	// In order to maximize the angle between old and new velocity, the angle between
	// acceleration vector and new velocity must be 90 degrees, meaning that I can
	// easily calculate the desired angle using simple cosine formula. The angle from 
	// old velocity to acceleration (which is what we actually want to return) is simply 
	// 90 degrees larger, so I'm doing some questionable trig math here to achieve this lol.
	float cosAng = -maxAccel / velocity.Length2D();
	if (cosAng < -1) cosAng = 0;

	return acosf(cosAng);
}


// get horizontal angle of wishdir that does correct thing based on given parameters
// angle is relative to your current velocity direction.
float AutoStrafeTool::GetStrafeAngle(const TasPlayerInfo &player, AutoStrafeParams &params) {
	Vector velocity = GetGroundFrictionVelocity(player);
	FOR_TAS_SCRIPT_VERSIONS_UNTIL(7) {
		velocity = player.velocity;
	}
	float speedDiff = params.strafeSpeed.speed - velocity.Length2D();
	if (absOld(speedDiff) < 0.001) speedDiff = 0;

	int turningDir = GetTurningDirection(player, params.strafeDir.angle);

	float ang = 0;
	bool passedTargetSpeed = false;

	if (speedDiff > 0) {
		ang = GetFastestStrafeAngle(player) * turningDir;
	} else if (speedDiff < 0) {
		ang = GetTurningStrafeAngle(player) * turningDir;
	}

	// check if the velocity is about to reach its target.
	if (speedDiff != 0) {
		float velAngle = TasUtils::GetVelocityAngles(&player).x;

		FOR_TAS_SCRIPT_VERSIONS_SINCE(6) {
			if (velocity.Length2D() == 0) {
				velAngle = params.strafeDir.angle;
			}
		}

		float correctAng = (DEG2RAD(velAngle) + ang) - DEG2RAD(player.angles.y);
		float forwardmove = cosf(correctAng);
		float sidemove = -sinf(correctAng);

		FOR_TAS_SCRIPT_VERSIONS_UNTIL(2) {
			forwardmove = cosOld(ang);
			sidemove = sinOld(ang);
		}

		forwardmove *= params.force;
		sidemove *= params.force;

		float predictedVel = GetVelocityAfterMove(player, forwardmove, sidemove).Length2D();
		if ((speedDiff > 0 && predictedVel > params.strafeSpeed.speed) || (speedDiff < 0 && predictedVel < params.strafeSpeed.speed)) {
			passedTargetSpeed = true;
		}
	}

	if (passedTargetSpeed || speedDiff == 0) {
		ang = GetTargetStrafeAngle(player, params.strafeSpeed.speed, turningDir);
	}

	return ang;
}

// returns 1 or -1 depending on what direction player should strafe (right and left accordingly)
int AutoStrafeTool::GetTurningDirection(const TasPlayerInfo &pInfo, float desAngle) {
	float velAngle = TasUtils::GetVelocityAngles(&pInfo).x;

	FOR_TAS_SCRIPT_VERSIONS_SINCE(6) {
		if (pInfo.velocity.Length2D() == 0) {
			velAngle = desAngle;
		}
	}

	float diff = desAngle - velAngle;
	if (absOld(diff - 360) < absOld(diff)) diff -= 360;
	if (absOld(diff + 360) < absOld(diff)) diff += 360;

	if (this->shouldFollowLine) {
		// we've reached our line!
		// for newer script versions, disable veccam - we don't need it anymore
		// BUT there is a better solution in a newer version lol
		FOR_TAS_SCRIPT_VERSIONS_SINCE(3) {
			if (params.strafeType == VECTORIAL_CAM) {
				params.strafeType = VECTORIAL;
				this->switchedFromVeccam = true;
			}
		}

		// check if deviated too far from the line
		// using the math from max angle change strafing to determine whether
		// line following is too "wobbly"
		Vector velocity = GetGroundFrictionVelocity(pInfo);
		float maxAccel = GetMaxAccel(pInfo, Vector(0, 1) * params.force);
		float maxRotAng = RAD2DEG(asinf(maxAccel / velocity.Length2D()));

		// scale maxRotAng by surfaceFriction and make it slightly bigger, as the range
		// is often surpassed by slowfly and some other shit I wasn't able to isolate
		FOR_TAS_SCRIPT_VERSIONS_SINCE(6) {
			maxRotAng *= 2.0 / pInfo.surfaceFriction;
		}

		if (absOld(diff) > maxRotAng) {
			this->shouldFollowLine = false;
			FOR_TAS_SCRIPT_VERSIONS_SINCE(6) {
				if (this->switchedFromVeccam) {
					params.strafeType = VECTORIAL_CAM;
				}
			}
			this->switchedFromVeccam = false;
			return GetTurningDirection(pInfo, desAngle);
		}

		// figure out on which side of line we're in, then return angle depending on that
		float desAngleRad = DEG2RAD(desAngle);
		Vector flForward(cosOld(desAngleRad), sinOld(desAngleRad));
		Vector flRight(flForward.y, -flForward.x);
		Vector vel = pInfo.velocity;
		Vector ppos = pInfo.position + vel * pInfo.ticktime;

		// translate the follow point so it's closer to player's position
		Vector flPoint = this->followLinePoint;
		flPoint = flPoint + flForward + flForward * (ppos - flPoint).Dot(flForward);

		// return direction based on what side of the line player is on
		float relPos = (ppos - flPoint).Normalize().Dot(flRight);
		return relPos < 0 ? -1 : 1;
	} else {
		// figure out the strafe direction based on a difference in angles
		float turnDir = 1;
		if (desAngle < -180.0f) {
			turnDir = -1;
		} else if (desAngle <= 180.0f) {
			if (diff < 0) turnDir = -1;
		}

		bool speedLockAvoided = false;

		FOR_TAS_SCRIPT_VERSIONS_SINCE(4) {
			// prevent losing acceleration speed from speedlock
			float airConLimit = 300.0f;
			FOR_TAS_SCRIPT_VERSIONS_SINCE(7) {
				if (sar_aircontrol.GetBool() && server->AllowsMovementChanges()) {
					airConLimit = INFINITY;
				}
			}

			bool shouldPreventSpeedLock = params.antiSpeedLock 
				&& pInfo.velocity.Length2D() < params.strafeSpeed.speed 
				&& pInfo.velocity.Length2D() >= airConLimit;

			FOR_TAS_SCRIPT_VERSIONS_SINCE(5) {
				shouldPreventSpeedLock = shouldPreventSpeedLock && !pInfo.willBeGrounded;
			}

			if (shouldPreventSpeedLock) {
				Vector wishDir = Vector(0, 1) * params.force;
				float maxSpeed = GetMaxSpeed(pInfo, wishDir);
				float maxAccel = GetMaxAccel(pInfo, wishDir);

				float speedCappedVel = (maxSpeed - maxAccel);

				// we're on a speedcapped bumpy road, so we're making sure to detour into speedy highway
				if (fabsf(pInfo.velocity.x) > speedCappedVel && fabsf(pInfo.velocity.y) > speedCappedVel) {
					float velAngle = atan2f(pInfo.velocity.y, pInfo.velocity.x);
					float cardinalDir = -sinf(velAngle * 4);
					cardinalDir = (cardinalDir == 0) ? turnDir : (cardinalDir / fabsf(cardinalDir));
					turnDir = cardinalDir;
					speedLockAvoided = true;
				}
			}
		}

		// reached the angle. follow the line from this point.
		if (this->lastTurnDir != 0 && this->lastTurnDir != turnDir && !speedLockAvoided) {
			FollowLine(pInfo);
		}

		// last turn is used to detect reaching line. avoid false detection from avoiding speedlock
		bool shouldRecordLastTurnDirection = !speedLockAvoided;
		FOR_TAS_SCRIPT_VERSIONS_UNTIL(5) {
			shouldRecordLastTurnDirection = true;
		}
		if (shouldRecordLastTurnDirection) this->lastTurnDir = turnDir;

		return turnDir;
	}
}

// enables line following and stores point needed for it
void AutoStrafeTool::FollowLine(const TasPlayerInfo &pInfo) {
	this->shouldFollowLine = true;
	this->followLinePoint = pInfo.position;
}


std::shared_ptr<TasToolParams> AutoStrafeTool::ParseParams(std::vector<std::string> vp) {
	AutoStrafeType type = VECTORIAL;
	AutoStrafeDirection dir = {CURRENT, false, 0};
	float maxSpeed = Variable("sv_maxvelocity").GetFloat() * 2.0f;
	AutoStrafeSpeed speed = {SPECIFIED, maxSpeed};
	bool noPitchLock = false;
	bool antiSpeedLock = true;
	float turnRate = 1.0f;
	float force = 1.0f;

	if (vp.size() == 0) {
		return std::make_shared<AutoStrafeParams>();
	}

	for (std::string param : vp) {
		//type
		if (param == "off") {
			return std::make_shared<AutoStrafeParams>();
		} else if (param == "vec") {
			type = VECTORIAL;
		} else if (param == "ang") {
			type = ANGULAR;
		} else if (param == "veccam") {
			type = VECTORIAL_CAM;
		}

		//speed
		else if (param == "max") {
			speed.type = SPECIFIED;
			speed.speed = maxSpeed;  // as long as it's higher than max speed times square root of 2, we should be fine?
		} else if (param == "keep") {
			speed.type = CURRENT;
		} else if (param.size() > 3 && param.substr(param.size() - 3, 3) == "ups") {
			speed.type = SPECIFIED;
			speed.speed = TasParser::toFloat(param.substr(0, param.size() - 3));
		} else if (param == "min") {
			speed.type = SPECIFIED;
			speed.speed = 0.0f;
		}

		//dir (using large numbers for left and right because angle is clamped to range -180 and 180)
		else if (param == "forwardvel") {
			dir.type = CURRENT;
			dir.useVelAngle = true;
		} else if (param == "forward") {
			dir.type = CURRENT;
			dir.useVelAngle = false;
		} else if (param == "left") {
			dir.type = SPECIFIED;
			dir.angle = 10000.0f;
		} else if (param == "right") {
			dir.type = SPECIFIED;
			dir.angle = -10000.0f;
		} else if (param.size() > 3 && param.substr(param.size() - 3, 3) == "deg") {
			dir.type = SPECIFIED;
			dir.angle = TasParser::toFloat(param.substr(0, param.size() - 3));
		}

		else if (param == "nopitchlock") {
			noPitchLock = true;
		}

		else if (param == "letspeedlock") {
			antiSpeedLock = false;
		}

		//named value parameters
		else if (param.find('=') != std::string::npos) {
			size_t eqPos = param.find('=');
			std::string key = param.substr(0, eqPos);
			std::string value = param.substr(eqPos + 1);
			
			if (key == "turnrate") {
				turnRate = TasParser::toFloat(value);
			} else if (key == "force") {
				force = TasParser::toFloat(value);
			} else if (key == "velocity" || key == "vel") {
				speed.type = SPECIFIED;
				speed.speed = TasParser::toFloat(value);
			} else if (key == "angle" || key == "ang") {
				dir.angle = SPECIFIED;
				dir.angle = TasParser::toFloat(value);
			} else {
				throw TasParserException(Utils::ssprintf("Unknown named parameter for tool %s: %s", this->GetName(), key.c_str()));
			}
		}

		//unknown parameter...
		else 
			throw TasParserException(Utils::ssprintf("Bad parameter for tool %s: %s", this->GetName(), param.c_str()));
	}

	return std::make_shared<AutoStrafeParams>(type, dir, speed, noPitchLock, antiSpeedLock, turnRate, force);
}
