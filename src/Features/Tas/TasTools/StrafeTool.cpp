#include "StrafeTool.hpp"
#include "../TasParser.hpp"

#include "Modules/Server.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Console.hpp"
#include "Utils/SDK.hpp"

#include "TasUtils.hpp"

AutoStrafeTool autoStrafeTool("strafe");

void AutoStrafeTool::Apply(TasFramebulk& fb, const TasPlayerInfo& pInfo)
{
    auto asParams = std::static_pointer_cast<AutoStrafeParams>(params);

    if (asParams->enabled) {
        float velAngle = TasUtils::GetVelocityAngles(&pInfo).x;
        float angle = velAngle + this->GetStrafeAngle(pInfo, *asParams, fb);

        if (asParams->strafeType == AutoStrafeType::VECTORIAL) {
            angle = DEG2RAD(angle - pInfo.angles.y);
            fb.moveAnalog.x = -sinf(angle);
            fb.moveAnalog.y = cosf(angle);
        } else if (asParams->strafeType == AutoStrafeType::ANGULAR) {
            float lookAngle = RAD2DEG(atan2f(fb.moveAnalog.x, fb.moveAnalog.y));

            QAngle newAngle = { 0, angle + lookAngle, 0 };
            fb.viewAnalog.x = newAngle.y - pInfo.angles.y;
        }

        if (asParams->strafeDir.type == AutoStrafeParamType::STRAIGHT) {
            asParams->direction *= -1;
        }
    }

    if (this->updated) {

        //TODO: update parameters that has type CURRENT here!

        this->updated = false;
    }
}

AutoStrafeTool* AutoStrafeTool::GetTool()
{
    return &autoStrafeTool;
}

std::shared_ptr<TasToolParams> AutoStrafeTool::ParseParams(std::vector<std::string> vp)
{
    AutoStrafeType type = VECTORIAL;
    AutoStrafeDirection dir{ STRAIGHT, 0 };
    AutoStrafeSpeed speed = { SPECIFIED, 10000.0f };
    bool turningPriority = false;
    int direction = 1;

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
            speed.speed = 10000.0f;
        } else if (param == "maintain") {
            speed.type = CURRENT;
        } else if (param.size() > 3 && param.substr(param.size() - 3, 3) == "ups") {
            speed.type = SPECIFIED;
            speed.speed = TasParser::toFloat(param.substr(0, param.size() - 3));
        }

        /*//dir (using large numbers for left and right because angle is clamped to range -180 and 180)
        else if (param == "current") {
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
        }*/

        else if (param == "straight") {
            dir.type = STRAIGHT;
        } else if (param == "left") {
            dir.type = TURN;
            direction = -1;
        } else if (param == "right") {
            dir.type = TURN;
            direction = 1;
        }

        //unknown parameter...
        else {
        
        }
    }

    return std::make_shared<AutoStrafeParams>(type, dir, speed, turningPriority, direction);
}

void AutoStrafeTool::Reset()
{
    params = std::make_shared<AutoStrafeParams>();
}



float AutoStrafeTool::GetStrafeAngle(const TasPlayerInfo& player, AutoStrafeParams &params, const TasFramebulk fb)
{
    float frametime = server->gpGlobals->frametime; // A time for one frame to pass

    // Getting player's friction
    float friction = sv_friction.GetFloat() * player.surfaceFriction;

    // Ground friction
    Vector velocity = player.velocity;
    Vector lambda = velocity;

    if (player.grounded) {
        if (velocity.Length2D() >= sv_stopspeed.GetFloat()) {
            lambda = lambda * (1.0f - frametime * friction);
        } else if (velocity.Length2D() >= std::fmax(0.1, frametime * sv_stopspeed.GetFloat() * friction)) {
            Vector normalizedVelocity = velocity;
            Math::VectorNormalize(normalizedVelocity);
            lambda = lambda + (normalizedVelocity * (frametime * sv_stopspeed.GetFloat() * friction)) * -1; // lambda -= v * tau * stop * friction
        } else {
            lambda = lambda * 0;
        }
    }

    // Getting M

    float forwardMove = fb.moveAnalog.x * 175.f;
    float sideMove = fb.moveAnalog.y * 175.f;

    float duckMultiplier = (player.grounded && player.ducked) ? 1.0f / 3.0f : 1.0f;

    float stateLen = sqrt(forwardMove * forwardMove + sideMove * sideMove);
    float F = forwardMove * player.maxSpeed * duckMultiplier / stateLen;
    float S = sideMove * player.maxSpeed * duckMultiplier / stateLen;

    float M = std::fminf(player.maxSpeed, sqrt(F * F + S * S));

    // Getting other stuff
    float A = (player.grounded) ? sv_accelerate.GetFloat() : sv_airaccelerate.GetFloat() / 2;
    float L = (player.grounded) ? M : std::fminf(60, M);

    // Getting the most optimal angle
    float cosTheta;
    if (params.strafeDir.type != AutoStrafeParamType::STRAIGHT && !player.grounded) {
        cosTheta = (player.surfaceFriction * frametime * M * A) / (2 * velocity.Length2D());
    } else if (params.strafeDir.type != AutoStrafeParamType::STRAIGHT && player.grounded) {
        cosTheta = std::cos(velocity.Length2D() * velocity.Length2D() - std::pow(player.surfaceFriction * frametime * M * A, 2) - lambda.Length2D() * lambda.Length2D()) / (2 * player.surfaceFriction * frametime * M * A * lambda.Length2D());
    } else {
        cosTheta = (L - player.surfaceFriction * frametime * M * A) / lambda.Length2D();
    }


    if (cosTheta < 0)
        cosTheta = M_PI_F / 2;
    if (cosTheta > 1)
        cosTheta = 0;

    float theta;
    if (params.strafeDir.type != AutoStrafeParamType::STRAIGHT) {
        theta = (M_PI_F - acosf(cosTheta)) * ((params.direction > 0) ? -1 : 1);
    } else {
        theta = acosf(cosTheta) * params.direction;
    }

    return RAD2DEG(theta);
}

Vector AutoStrafeTool::PredictNextVector(const TasPlayerInfo& player, float angle)
{
    Vector v = { 0, 0, 0 };
    return v;
}