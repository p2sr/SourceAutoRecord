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
        this->reachedAngle = false;
        this->reachedVelocity = false;

        if (asParams->strafeDir.type == CURRENT) {
            asParams->strafeDir.angle = velAngle;
            MarkAngleReached(pInfo);
        }

        if (asParams->strafeSpeed.type == CURRENT) {
            asParams->strafeSpeed.speed = pInfo.velocity.Length2D();
            this->reachedVelocity = true;
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
        float lookAngle = RAD2DEG(atan2f(fb.moveAnalog.x, fb.moveAnalog.y));

        QAngle newAngle = { 0, angle + lookAngle, 0 };
        fb.viewAnalog.x = newAngle.y - pInfo.angles.y;
    }
    
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
            vel = Vector(0, 0, 0);
        }
    }

    return vel;
}

// returns max speed value which is used by autostrafer math
float AutoStrafeTool::GetMaxSpeed(const TasPlayerInfo& player, Vector wishDir, bool notAired)
{
    float duckMultiplier = (player.grounded && player.ducked) ? (1.0f / 3.0f) : 1.0f;

    wishDir.y *= cl_forwardspeed.GetFloat();
    wishDir.x *= cl_sidespeed.GetFloat();

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

// returns the predicted velocity in the next tick
Vector AutoStrafeTool::GetVelocityAfterMove(const TasPlayerInfo& player, Vector wishDir)
{
    Vector velocity = GetGroundFrictionVelocity(player);

    if (wishDir.Length2D() == 0) return velocity;

    if (!player.grounded) wishDir.y *= cos(player.angles.x);

    float maxSpeed = GetMaxSpeed(player, wishDir);
    float maxAccel = GetMaxAccel(player, wishDir);

    float pitch = DEG2RAD(player.angles.y);
    Vector rotWishDir(sin(pitch) * wishDir.x + cos(pitch) * wishDir.y, -cos(pitch) * wishDir.x + sin(pitch) * wishDir.y);
    float accelDiff = maxSpeed - velocity.Dot(rotWishDir.Normalize());

    if (accelDiff <= 0) return velocity;

    float accelForce = fminf(maxAccel, accelDiff);

    return velocity + rotWishDir.Normalize() * accelForce;
}

// get horizontal angle of wishdir that would give you the fastest acceleration
// angle is relative to your current velocity direction.
float AutoStrafeTool::GetFastestStrafeAngle(const TasPlayerInfo& player)
{
    return 0;
}

// get horizontal angle of wishdir that would maintain current velocity
// angle is relative to your current velocity direction.
float AutoStrafeTool::GetMaintainStrafeAngle(const TasPlayerInfo& player)
{
    return 0;
}

// get horizontal angle of wishdir that does correct thing based on given parameters
// angle is relative to your current velocity direction.
float AutoStrafeTool::GetStrafeAngle(const TasPlayerInfo& player, AutoStrafeParams &params)
{
    return 0;
}

//change boolean, but also generates a line an autostrafer will follow later on.
void AutoStrafeTool::MarkAngleReached(const TasPlayerInfo& pInfo)
{
    this->reachedAngle = true;

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

        //priority
        else if (param == "turn") {
            turningPriority = true;
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