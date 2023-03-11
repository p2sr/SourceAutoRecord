#include "StrafeTool.hpp"

#include "../TasParser.hpp"
#include "AutoJumpTool.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "TasUtils.hpp"
#include "Utils/SDK.hpp"

#include <cfloat>

AutoStrafeTool autoStrafeTool[2] = {{0}, {1}};

void AutoStrafeTool::Apply(TasFramebulk &fb, const TasPlayerInfo &rawPInfo) {
	if (!params.enabled)
		return;

	//create fake player info for a sake of values being correct
	TasPlayerInfo pInfo = rawPInfo;

	// NOTE: this check is kinda dumb, but it prevents a tool order
	// dependency where strafe has to know about autojump's inputs
	// I feel like this could live somewhere else and be a bit less dumb
	if (autoJumpTool[this->slot].GetCurrentParams().enabled) {
		// if autojump is enabled, we're never grounded.
		pInfo.grounded = false;
	}

	// when not grounded, air acceleration is not optimal if pitch is not
	// in a range between -30 and 30 degrees (both exclusive). making sure
	// it's in the right range, unless specifically asked to not do it.
	bool compensatePitchMult = false;
	if (!pInfo.grounded && fabsf(pInfo.angles.x - fb.viewAnalog.y) >= 30.0f) {
		if (!params.noPitchLock) {
			float signAng = tasPlayer->GetScriptVersion(slot) >= 6 ? (pInfo.angles.x - fb.viewAnalog.y) : pInfo.angles.x;
			float diff = pInfo.angles.x - (29.9999f * (signAng / absOld(signAng)));
			fb.viewAnalog.y = diff;
		} else {
			compensatePitchMult = true;
		}
	}

	// adjusting fake pinfo to have proper angles (after rotation)
	pInfo.angles.y -= fb.viewAnalog.x;
	pInfo.angles.x -= fb.viewAnalog.y;

	float velAngle = TasUtils::GetVelocityAngles(&pInfo).x;
	if (pInfo.velocity.Length2D() == 0 && tasPlayer->GetScriptVersion(slot) >= 6) {
		if (this->updated && params.strafeDir.useVelAngle) {
			velAngle = pInfo.angles.y;
		} else {
			velAngle = params.strafeDir.angle;
		}
	}

	// update parameters that has type CURRENT
	if (this->updated) {
		this->shouldFollowLine = false;
		this->lastTurnDir = 0;
		this->switchedFromVeccam = false;

		if (params.strafeDir.type == CURRENT) {
			if (params.strafeDir.useVelAngle) {
				params.strafeDir.angle = velAngle;
				FollowLine(pInfo);
			} else {
				params.strafeDir.angle = rawPInfo.angles.y;  //	using real angles instead of fake ones here.
			}
		}

		if (params.strafeSpeed.type == CURRENT) {
			params.strafeSpeed.speed = pInfo.velocity.Length2D();
		}

		this->updated = false;
	}


	float angle = velAngle + RAD2DEG(this->GetStrafeAngle(pInfo, params));

	// applying the calculated angle depending on the type of strafing
	if (params.strafeType == AutoStrafeType::VECTORIAL) {
		float moveAngle = DEG2RAD(angle - pInfo.angles.y);
		fb.moveAnalog.x = -sinf(moveAngle);
		fb.moveAnalog.y = cosf(moveAngle);
		if (pInfo.onSpeedPaint) fb.moveAnalog.x *= 2;
	} else if (params.strafeType == AutoStrafeType::VECTORIAL_CAM) {
		float lookAngle = this->shouldFollowLine ? params.strafeDir.angle : velAngle;
		if (tasPlayer->GetScriptVersion(slot) >= 6) {
			fb.viewAnalog.x -= lookAngle - pInfo.angles.y;
		} else {
			fb.viewAnalog.x = -(lookAngle - pInfo.angles.y);
		}
		float moveAngle = DEG2RAD(angle - lookAngle);
		fb.moveAnalog.x = -sinf(moveAngle);
		fb.moveAnalog.y = cosf(moveAngle);
		if (pInfo.onSpeedPaint) fb.moveAnalog.x *= 2;
	} else if (params.strafeType == AutoStrafeType::ANGULAR) {
		//making sure moveAnalog is always at maximum value.
		if (fb.moveAnalog.Length2D() == 0) {
			fb.moveAnalog.y = 1;
		} else {
			fb.moveAnalog = fb.moveAnalog.Normalize();
		}

		float lookAngle = RAD2DEG(atan2f(fb.moveAnalog.x, fb.moveAnalog.y));

		QAngle newAngle = {0, angle + lookAngle, 0};
		fb.viewAnalog.x -= newAngle.y - pInfo.angles.y;
	}

	//pitch lock isn't used. try to compensate the pitch movement multiplication by dividing it now
	if (compensatePitchMult) {
		fb.moveAnalog.y /= cosOld(DEG2RAD(pInfo.angles.x));
	}

}

// returns player's velocity after its been affected by ground friction
Vector AutoStrafeTool::GetGroundFrictionVelocity(const TasPlayerInfo &player) {
	// Getting player's friction
	// it is important to include a right value, as it is modified on a slowfly effect.
	float friction = sv_friction.GetFloat() * player.surfaceFriction;

	Vector vel = player.velocity;

	if (player.grounded) {
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
	float duckMultiplier = (player.grounded && player.ducked) ? (1.0f / 3.0f) : 1.0f;
	wishDir.y *= player.maxSpeed;
	wishDir.x *= player.maxSpeed;
	float maxSpeed = fminf(player.maxSpeed, wishDir.Length2D()) * duckMultiplier;
	float maxAiredSpeed = (player.grounded || notAired) ? maxSpeed : fminf(60, maxSpeed);

	return maxAiredSpeed;
}


float AutoStrafeTool::GetMaxAccel(const TasPlayerInfo &player, Vector wishDir) {
	bool aircon2 = sar_aircontrol.GetInt() == 2 && server->AllowsMovementChanges();
	float accel = (player.grounded) ? sv_accelerate.GetFloat() : aircon2 ? sv_airaccelerate.GetFloat() : sv_paintairacceleration.GetFloat();
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
		if (!player.grounded) {
			if (absOld(player.angles.x) >= 30.0f) {
				wishDir.y *= cosOld(DEG2RAD(player.angles.x));
			}
		}
	}

	//rotating wishDir
	float yaw = DEG2RAD(player.angles.y);
	wishDir = Vector(sinOld(yaw) * wishDir.x + cosOld(yaw) * wishDir.y, -cosOld(yaw) * wishDir.x + sinOld(yaw) * wishDir.y);

	// air control limit
	float airConLimit = (sar_aircontrol.GetBool() && server->AllowsMovementChanges()) ? INFINITY : 300;
	if (!player.grounded && player.velocity.Length2D() > airConLimit) {
		if (absOld(player.velocity.x) > airConLimit * 0.5 && player.velocity.x * wishDir.x < 0) {
			wishDir.x = 0;
		}
		if (absOld(player.velocity.y) > airConLimit * 0.5 && player.velocity.y * wishDir.y < 0) {
			wishDir.y = 0;
		}
	}

	return wishDir;
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

	Vector wishDir(0, 1);
	float maxSpeed = GetMaxSpeed(player, wishDir);
	float maxAccel = GetMaxAccel(player, wishDir);

	// finding the most optimal angle. 
	// formula shamelessly taken from https://www.jwchong.com/hl/movement.html
	float cosAng = (maxSpeed - maxAccel) / velocity.Length2D();

	return acosf(fminf(fmaxf(cosAng,0.0f),1.0f));
}

// get horizontal angle of wishdir that would achieve given velocity
// angle is relative to your current velocity direction.
float AutoStrafeTool::GetTargetStrafeAngle(const TasPlayerInfo &player, float targetSpeed) {
	Vector vel = GetGroundFrictionVelocity(player);

	if (vel.Length2D() == 0) return 0;

	Vector wishDir(0, 1);
	//float maxSpeed = GetMaxSpeed(player, wishDir);
	float maxAccel = GetMaxAccel(player, wishDir);

	// Assuming that it is possible to achieve a velocity of a given length,
	// I'm using a law of cosines to get the right angle for acceleration.
	float cosAng = (powOld(vel.Length2D(), 2) + powOld(maxAccel, 2) - powOld(targetSpeed, 2)) / (2.0f * vel.Length2D() * maxAccel);

	// Also, questionable trig to get the right angle lol.
	return acosf(-cosAng);
}

// get horizontal angle of wishdir that would give the biggest turning angle in given tick
// angle is relative to your current velocity direction.
float AutoStrafeTool::GetTurningStrafeAngle(const TasPlayerInfo &player) {
	Vector velocity = GetGroundFrictionVelocity(player);

	if (velocity.Length2D() == 0) return 0;

	Vector wishDir(0, 1);
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
	float speed = player.velocity.Length2D();

	float speedDiff = params.strafeSpeed.speed - speed;
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

		float forwardmove = cosOld(ang);
		float sidemove = sinOld(ang);

		// forwardmove and sidemove were calculated incorrectly in v2 and older
		// fix for newer, keep for older for backwards compability
		if (tasPlayer->GetScriptVersion(slot) >= 3) {
			float velAngle = TasUtils::GetVelocityAngles(&player).x;
			if (player.velocity.Length2D() == 0 && tasPlayer->GetScriptVersion(slot) >= 6) velAngle = params.strafeDir.angle;
			float correctAng = (DEG2RAD(velAngle) + ang) - DEG2RAD(player.angles.y);
			forwardmove = cosf(correctAng);
			sidemove = -sinf(correctAng);
		}

		float predictedVel = GetVelocityAfterMove(player, forwardmove, sidemove).Length2D();
		if ((speedDiff > 0 && predictedVel > params.strafeSpeed.speed) || (speedDiff < 0 && predictedVel < params.strafeSpeed.speed)) {
			passedTargetSpeed = true;
		}
	}

	if (passedTargetSpeed || speedDiff == 0) {
		ang = GetTargetStrafeAngle(player, params.strafeSpeed.speed) * turningDir;
	}

	// TODO: handle air control limit correctly


	return ang;
}

// returns 1 or -1 depending on what direction player should strafe (right and left accordingly)
int AutoStrafeTool::GetTurningDirection(const TasPlayerInfo &pInfo, float desAngle) {
	float velAngle = TasUtils::GetVelocityAngles(&pInfo).x;
	if (pInfo.velocity.Length2D() == 0 && tasPlayer->GetScriptVersion(slot) >= 6) velAngle = desAngle;
	float diff = desAngle - velAngle;
	if (absOld(diff - 360) < absOld(diff)) diff -= 360;
	if (absOld(diff + 360) < absOld(diff)) diff += 360;

	if (this->shouldFollowLine) {
		// we've reached our line!
		// for newer script versions, disable veccam - we don't need it anymore
		// BUT there is a better solution in a newer version lol
		if (tasPlayer->GetScriptVersion(slot) >= 3) {
			if (params.strafeType == VECTORIAL_CAM) {
				params.strafeType = VECTORIAL;
				this->switchedFromVeccam = true;
			}
		}

		// check if deviated too far from the line
		// using the math from max angle change strafing to determine whether
		// line following is too "wobbly"
		Vector velocity = GetGroundFrictionVelocity(pInfo);
		float maxAccel = GetMaxAccel(pInfo, Vector(0, 1));
		float maxRotAng = RAD2DEG(asinf(maxAccel / velocity.Length2D()));

		// scale maxRotAng by surfaceFriction and make it slightly bigger, as the range
		// is often surpassed by slowfly and some other shit I wasn't able to isolate
		if (tasPlayer->GetScriptVersion(slot) >= 6) maxRotAng *= 2.0 / pInfo.surfaceFriction;

		if (absOld(diff) > maxRotAng) {
			this->shouldFollowLine = false;
			if (tasPlayer->GetScriptVersion(slot) >= 6 && this->switchedFromVeccam) {
				params.strafeType = VECTORIAL_CAM;
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

		// prevent losing acceleration speed from speedlock on newer versions
		// also remember that speedlock exists only when midair in versions 5 or newer
		// also remember that aircontrol removes speedlock in versions 7 or newer
		float airConLimit = (sar_aircontrol.GetBool() && server->AllowsMovementChanges()) ? INFINITY : 300.0f;
		if (params.antiSpeedLock && tasPlayer->GetScriptVersion(slot) >= 4 && (!pInfo.grounded || tasPlayer->GetScriptVersion(slot) < 5)) {
			if (pInfo.velocity.Length2D() < params.strafeSpeed.speed && pInfo.velocity.Length2D() >= (tasPlayer->GetScriptVersion(slot) >= 7 ? airConLimit : 300.0f)) {

				Vector wishDir(0, 1);
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
		if (tasPlayer->GetScriptVersion(slot) < 6 || !speedLockAvoided) this->lastTurnDir = turnDir;
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
	AutoStrafeDirection dir{CURRENT, false, 0};
	AutoStrafeSpeed speed = {SPECIFIED, 10000.0f};
	bool noPitchLock = false;
	bool antiSpeedLock = true;

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
			speed.speed = 10000.0f;  // as long as it's higher than max speed times square root of 2, we should be fine?
		} else if (param == "keep") {
			speed.type = CURRENT;
		} else if (param.size() > 3 && param.substr(param.size() - 3, 3) == "ups") {
			speed.type = SPECIFIED;
			speed.speed = TasParser::toFloat(param.substr(0, param.size() - 3));
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

		//unknown parameter...
		else 
			throw TasParserException(Utils::ssprintf("Bad parameter for tool %s: %s", this->GetName(), param.c_str()));
	}

	return std::make_shared<AutoStrafeParams>(type, dir, speed, noPitchLock, antiSpeedLock);
}
