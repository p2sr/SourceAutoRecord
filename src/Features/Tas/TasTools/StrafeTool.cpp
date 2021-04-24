#include "StrafeTool.hpp"
#include "../TasParser.hpp"

#include "Modules/Server.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Console.hpp"
#include "Utils/SDK.hpp"

#include "TasUtils.hpp"

AutoStrafeTool autoStrafeTool("strafe");

void AutoStrafeTool::Apply(TasFramebulk& fb, const TasPlayerInfo& pInfo)
{
    auto asParams = std::static_pointer_cast<AutoStrafeParams>(params);

    if (!asParams->enabled)
        return;

    float velAngle = TasUtils::GetVelocityAngles(&pInfo).x;

    // update parameters that has type CURRENT
    if (this->updated) {
        this->shouldFollowLine = false;

        if (asParams->strafeDir.type == CURRENT) {
            asParams->strafeDir.angle = velAngle;
            FollowLine(pInfo);
        }

        if (asParams->strafeSpeed.type == CURRENT) {
            asParams->strafeSpeed.speed = pInfo.velocity.Length2D();
        }

        this->updated = false;
    }

    
    float angle = velAngle + this->GetStrafeAngle(pInfo, *asParams);

    // applying the calculated angle depending on the type of strafing
    if (asParams->strafeType == AutoStrafeType::VECTORIAL) {
        angle = DEG2RAD(angle - pInfo.angles.y);
        fb.moveAnalog.x = -sinf(angle);
        fb.moveAnalog.y = cosf(angle);
    } else if (asParams->strafeType == AutoStrafeType::ANGULAR) {

        //making sure moveAnalog is always at maximum value.
        if (fb.moveAnalog.Length2D() == 0) {
            fb.moveAnalog.y = 1;
        } else {
            fb.moveAnalog = fb.moveAnalog.Normalize();
        }

        float lookAngle = RAD2DEG(atan2f(fb.moveAnalog.x, fb.moveAnalog.y));

        QAngle newAngle = { 0, angle + lookAngle, 0 };
        fb.viewAnalog.x = newAngle.y - pInfo.angles.y;
    }

    // forcing pitch to be 0
    fb.viewAnalog.y = -pInfo.angles.x;
    
}

// returns player's velocity after its been affected by ground friction
Vector AutoStrafeTool::GetGroundFrictionVelocity(const TasPlayerInfo& player)
{
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
float AutoStrafeTool::GetMaxSpeed(const TasPlayerInfo& player, Vector wishDir, bool notAired)
{
    // calculate max speed based on player inputs, grounded and ducking states.
    float duckMultiplier = (player.grounded && player.ducked) ? (1.0f / 3.0f) : 1.0f;
    wishDir.y *= player.maxSpeed;
    wishDir.x *= player.maxSpeed;
    float maxSpeed = fminf(player.maxSpeed, wishDir.Length2D()) * duckMultiplier;
    float maxAiredSpeed = (player.grounded || notAired) ? maxSpeed : fminf(60, maxSpeed);

    return maxAiredSpeed;
}


float AutoStrafeTool::GetMaxAccel(const TasPlayerInfo& player, Vector wishDir)
{
    float accel = (player.grounded) ? sv_accelerate.GetFloat() : sv_paintairacceleration.GetFloat();
    float realAccel = player.surfaceFriction * player.ticktime * GetMaxSpeed(player,wishDir,true) * accel;
    return realAccel;
}


Vector AutoStrafeTool::CreateWishDir(const TasPlayerInfo& player, float forwardMove, float sideMove)
{
    Vector wishDir(sideMove, forwardMove);
    if (wishDir.Length() > 1.f) {
        wishDir = wishDir.Normalize();
    }

    // forwardmove is affected by player pitch when in air
    if (!player.grounded) {
        wishDir.y *= cos(DEG2RAD(player.angles.x));
    }

    //rotating wishDir
    float yaw = DEG2RAD(player.angles.y);
    wishDir = Vector(sin(yaw) * wishDir.x + cos(yaw) * wishDir.y, -cos(yaw) * wishDir.x + sin(yaw) * wishDir.y);

    // air control limit
    float airConLimit = 300;
    if (!player.grounded && player.velocity.Length2D() > airConLimit) {
        if (abs(player.velocity.x) > airConLimit * 0.5 && player.velocity.x * wishDir.x < 0) {
            wishDir.x = 0;
        }
        if (abs(player.velocity.y) > airConLimit * 0.5 && player.velocity.y * wishDir.y < 0) {
            wishDir.y = 0;
        }
    }

    return wishDir;
}


// returns the predicted velocity in the next tick
Vector AutoStrafeTool::GetVelocityAfterMove(const TasPlayerInfo& player, float forwardMove, float sideMove)
{
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
float AutoStrafeTool::GetFastestStrafeAngle(const TasPlayerInfo& player)
{
    Vector velocity = GetGroundFrictionVelocity(player);

    if (velocity.Length2D() == 0) return 0;

    Vector wishDir(0, 1);
    float maxSpeed = GetMaxSpeed(player, wishDir);
    float maxAccel = GetMaxAccel(player, wishDir);

    //finding the most optimal angle
    float cosAng = (maxSpeed - maxAccel) / velocity.Length2D();

    if (cosAng < 0) cosAng = M_PI_F / 2;
    if (cosAng > 1) cosAng = 0;

    return acosf(cosAng);
}

// get horizontal angle of wishdir that would maintain current velocity
// angle is relative to your current velocity direction.
float AutoStrafeTool::GetMaintainStrafeAngle(const TasPlayerInfo& player)
{
    Vector vel = player.velocity;
    Vector gfVel = GetGroundFrictionVelocity(player);

    if (gfVel.Length2D() == 0) return 0;

    Vector wishDir(0, 1);
    float maxSpeed = GetMaxSpeed(player, wishDir);
    float maxAccel = GetMaxAccel(player, wishDir);

    float cosAng;

    if (!player.grounded)
        cosAng = maxAccel / (2 * vel.Length2D());
    else
        cosAng = cosf(powf(vel.Length2D(), 2) - powf(maxAccel, 2) - powf(gfVel.Length2D(), 2)) / (2 * maxAccel * gfVel.Length2D());

    if (cosAng < 0) cosAng = M_PI_F / 2;
    if (cosAng > 1) cosAng = 0;

    return (M_PI_F - acosf(cosAng));
}

// get horizontal angle of wishdir that would give the biggest turning angle in given tick
// angle is relative to your current velocity direction.
float AutoStrafeTool::GetTurningStrafeAngle(const TasPlayerInfo& player)
{
    Vector velocity = GetGroundFrictionVelocity(player);

    if (velocity.Length2D() == 0) return 0;

    Vector wishDir(0, 1);
    float maxSpeed = GetMaxSpeed(player, wishDir);
    float maxAccel = GetMaxAccel(player, wishDir);

    float cosAng = -maxAccel / velocity.Length2D();
    if (cosAng < -1) cosAng = 0;
    
    return acosf(cosAng);
}


// get horizontal angle of wishdir that does correct thing based on given parameters
// angle is relative to your current velocity direction.
float AutoStrafeTool::GetStrafeAngle(const TasPlayerInfo& player, AutoStrafeParams &params)
{
    float speed = player.velocity.Length2D();

    float speedDiff = params.strafeSpeed.speed - speed;
    if (abs(speedDiff) < 0.001) speedDiff = 0;

    float ang = 0;

    if (speedDiff == 0)
        ang = GetMaintainStrafeAngle(player);
    else if (speedDiff > 0)
        ang = GetFastestStrafeAngle(player);
    else if (speedDiff < 0)
        ang = GetTurningStrafeAngle(player);
 
    ang *= GetTurningDirection(player);

    // TODO: here, we need to check if the velocity is about to reach its target.
    // also check for limited air control conditions.

    return ang;
}

// returns 1 or -1 depending on what direction player should strafe (right and left accordingly)
int AutoStrafeTool::GetTurningDirection(const TasPlayerInfo& pInfo)
{
    if (this->shouldFollowLine) {
        //TODO: here figure out on which side of line we're in, then return angle depending on that
    } else {
        float turnDir = 1;

        //TODO: calculate correct difference between current and desired angle, then make a turning direction out of that.

        if (this->lastTurnDir != 0 && this->lastTurnDir != turnDir) {
            FollowLine(pInfo);
        }

        this->lastTurnDir = turnDir;
        return turnDir;
    }
}

// enables line following and stores point needed for it
void AutoStrafeTool::FollowLine(const TasPlayerInfo& pInfo)
{
    this->shouldFollowLine = true;
    this->followLinePoint = pInfo.position;
}



std::shared_ptr<TasToolParams> AutoStrafeTool::ParseParams(std::vector<std::string> vp)
{
    AutoStrafeType type = VECTORIAL;
    AutoStrafeDirection dir{ CURRENT, 0 };
    AutoStrafeSpeed speed = { SPECIFIED, 10000.0f };
    bool turningPriority = false;

    if (vp.size() == 0) {
        return std::make_shared<AutoStrafeParams>();
    }

    for (std::string param : vp) {
        //type
        if (param == "off") {
            return std::make_shared<AutoStrafeParams>();
        } else if (param == "vectorial") {
            type = VECTORIAL;
        } else if (param == "angular") {
            type = ANGULAR;
        }

        //speed
        else if (param == "max") {
            speed.type = SPECIFIED;
            speed.speed = 10000.0f; // as long as it's higher than max speed times square root of 2, we should be fine?
        } else if (param == "maintain") {
            speed.type = CURRENT;
        } else if (param.size() > 3 && param.substr(param.size() - 3, 3) == "ups") {
            speed.type = SPECIFIED;
            speed.speed = TasParser::toFloat(param.substr(0, param.size() - 3));
        }

        //dir (using large numbers for left and right because angle is clamped to range -180 and 180)
        else if (param == "straight") {
            dir.type = CURRENT;
        } else if (param == "left") {
            dir.type = SPECIFIED;
            dir.angle = 360.0f;
        } else if (param == "right") {
            dir.type = SPECIFIED;
            dir.angle = -360.0f;
        } else if (param.size() > 3 && param.substr(param.size() - 3, 3) == "deg") {
            dir.type = SPECIFIED;
            dir.angle = TasParser::toFloat(param.substr(0, param.size() - 3));
        }

        //unknown parameter...
        else {
            // do we even handle unknown parameters?!
            // mlugg is going to be pissed
        }
    }

    return std::make_shared<AutoStrafeParams>(type, dir, speed, turningPriority);
}

AutoStrafeTool* AutoStrafeTool::GetTool()
{
    return &autoStrafeTool;
}

void AutoStrafeTool::Reset()
{
    params = std::make_shared<AutoStrafeParams>();
}