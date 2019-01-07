#include "TasTools.hpp"

#include <cmath>
#include <cstring>

#include "Features/OffsetFinder.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "SAR.hpp"
#include "Utils.hpp"

TasTools* tasTools;

TasTools::TasTools()
    : propName("m_hGroundEntity")
    , propType(PropType::Handle)
    , wantToStrafe(0)
    , strafingDirection(0)
    , isVectorial(0)
    , strafeType(0)
    , isTurning(0)
    , acceleration({ 0, 0, 0 })
    , prevVelocity({ 0, 0, 0 })
    , prevTick(0)
{
    if (sar.game->version & (SourceGame_Portal | SourceGame_Portal2Engine)) {
        std::strncpy(this->className, "CPortal_Player", sizeof(this->className));
    } else if (sar.game->version & SourceGame_HalfLife2) {
        std::strncpy(this->className, "CHL2_Player", sizeof(this->className));
    } else {
        std::strncpy(this->className, "CBasePlayer", sizeof(this->className));
    }

    offsetFinder->ClientSide(this->className, this->propName, &this->propOffset);

    this->hasLoaded = true;
}
void TasTools::AimAtPoint(float x, float y, float z)
{
    Vector target = { y, x, z };
    Vector campos = client->GetAbsOrigin() + client->GetViewOffset();
    campos = { campos.y, campos.x, campos.z };

    // Axis in the game, need to know it to fix up:
    //              : G - D ; Av - Ar ; H - B
    // Rotation Axis:   x        z        y
    // Translation  :   y        x        z

    float xdis = target.x - campos.x;
    float ydis = target.z - campos.z;
    float zdis = target.y - campos.y;
    float xzdis = sqrtf(xdis * xdis + zdis * zdis);

    QAngle angles = { RAD2DEG(-atan2f(ydis, xzdis)), RAD2DEG(-(atan2f(-xdis, zdis))), 0 };

    engine->SetAngles(angles);
}
void* TasTools::GetPlayerInfo()
{
    auto player = client->GetPlayer();
    return (player) ? reinterpret_cast<void*>((uintptr_t)player + this->propOffset) : nullptr;
}
Vector TasTools::GetVelocityAngles()
{
    auto velocityAngles = client->GetLocalVelocity();
    if (velocityAngles.Length() == 0) {
        return { 0, 0, 0 };
    }

    Math::VectorNormalize(velocityAngles);

    float yaw = atan2f(velocityAngles.y, velocityAngles.x);
    float pitch = atan2f(velocityAngles.z, sqrtf(velocityAngles.y * velocityAngles.y + velocityAngles.x * velocityAngles.x));

    return { RAD2DEG(yaw), RAD2DEG(pitch), 0 };
}
Vector TasTools::GetAcceleration()
{
    auto curTick = engine->GetSessionTick();
    if (this->prevTick != curTick) {
        auto curVelocity = client->GetLocalVelocity();

        // z used to represent the combined x/y acceleration axis value
        this->acceleration.z = curVelocity.Length2D() - prevVelocity.Length2D();
        this->acceleration.x = std::abs(curVelocity.x) - std::abs(this->prevVelocity.x);
        this->acceleration.y = std::abs(curVelocity.y) - std::abs(this->prevVelocity.y);

        this->prevVelocity = curVelocity;
        this->prevTick = curTick;
    }

    return this->acceleration;
}
// Returns an absolute angle for perfect wish direction vector
float TasTools::GetStrafeAngle(CMoveData* pmove, int direction)
{
    float tau = 1 / host_framerate.GetFloat(); // A time for one frame to pass

    // Getting player's friction
    float player_friction = (*reinterpret_cast<float*>((uintptr_t)client->GetPlayer() + Offsets::m_flFriction));
    float friction = sv_friction.GetFloat() * player_friction * 1;

    // Creating lambda(v) - velocity after ground friction
    Vector velocity = client->GetLocalVelocity();
    Vector lambda = velocity;

    bool pressed_jump = ((pmove->m_nButtons & IN_JUMP) > 0 && (pmove->m_nOldButtons & IN_JUMP) == 0);
    bool grounded = pmove->m_vecVelocity.z == 0 && !pressed_jump;
    if (grounded) {
        if (velocity.Length2D() >= sv_stopspeed.GetFloat()) {
            lambda = lambda * (1.0f - tau * friction);
        } else if (velocity.Length2D() >= std::fmax(0.1, tau * sv_stopspeed.GetFloat() * friction)) {
            Vector normalized_velocity = velocity;
            Math::VectorNormalize(normalized_velocity);
            lambda = lambda + (normalized_velocity * (tau * sv_stopspeed.GetFloat() * friction)) * -1; // lambda -= v * tau * stop * friction
        } else {
            lambda = lambda * 0;
        }
    }

    // Getting M
    float F = pmove->m_flForwardMove / cl_forwardspeed.GetFloat();
    float S = pmove->m_flSideMove / cl_sidespeed.GetFloat();

    float stateLen = sqrt(F * F + S * S);
    float forwardMove = pmove->m_flForwardMove / stateLen;
    float sideMove = pmove->m_flSideMove / stateLen;
    float M = std::fminf(sv_maxspeed.GetFloat(), sqrt(forwardMove * forwardMove + sideMove * sideMove));

    // Getting other stuff
    float A = (grounded) ? sv_accelerate.GetFloat() : sv_airaccelerate.GetFloat() / 2;
    float L = (grounded) ? M : std::fminf(60, M);

    // Getting the most optimal angle
    float cosTheta;
    if (this->strafeType == 2 && !grounded) {
        cosTheta = (player_friction * tau * M * A) / (2 * velocity.Length2D());
    } else if (this->strafeType == 2 && grounded) {
        cosTheta = std::cos(velocity.Length2D() * velocity.Length2D() - std::pow(player_friction * tau * M * A, 2) - lambda.Length2D() * lambda.Length2D()) / (2 * player_friction * tau * M * A * lambda.Length2D());
    } else {
        cosTheta = (L - player_friction * tau * M * A) / lambda.Length2D();
    }

    if (cosTheta < 0)
        cosTheta = M_PI_F / 2;
    if (cosTheta > 1)
        cosTheta = 0;

    float theta;
    if (this->strafeType == 2) {
        theta = (M_PI_F - acosf(cosTheta)) * ((direction > 0) ? -1 : 1);
    } else {
        theta = acosf(cosTheta) * ((direction > 0) ? -1 : 1);
    }

    return this->GetVelocityAngles().x + RAD2DEG(theta);
}
// Angle set based on current wishdir
void TasTools::Strafe(CMoveData* pMove)
{
    if ((pMove->m_nButtons & 0b11000011000) > 0) {
        float angle = this->GetStrafeAngle(pMove, this->strafingDirection);
        float lookangle = RAD2DEG(atan2f(pMove->m_flSideMove, pMove->m_flForwardMove));

        QAngle newAngle = { 0, angle + lookangle, 0 };
        pMove->m_vecViewAngles = newAngle;
        pMove->m_vecAbsViewAngles = newAngle;
        engine->SetAngles(newAngle);
    }
}
// whishdir set based on current angle ("controller" inputs)
void TasTools::VectorialStrafe(CMoveData* pMove)
{
    // don't strafe if player is not holding any movement button
    // temporary fix, we should find a way to make the game think we're holding one
    if ((pMove->m_nButtons & 0b11000011000) > 0) {
        float angle = this->GetStrafeAngle(pMove, this->strafingDirection);
        angle = DEG2RAD(angle - pMove->m_vecAbsViewAngles.y);

        pMove->m_flForwardMove = cosf(angle) * cl_forwardspeed.GetFloat();
        pMove->m_flSideMove = -sinf(angle) * cl_sidespeed.GetFloat();
    }
}

// Commands

CON_COMMAND(sar_tas_aim_at_point, "sar_tas_aim_at_point <x> <y> <z> : Aims at point {x, y, z} specified.\n")
{
    if (!sv_cheats.GetBool()) {
        return console->Print("Cannot use sar_tas_aim_at_point without sv_cheats set to 1.\n");
    }

    if (args.ArgC() != 4) {
        return console->Print("sar_tas_aim_at_point <x> <y> <z> : Aims at point {x, y, z} specified.\n");
    }

    tasTools->AimAtPoint(static_cast<float>(std::atof(args[1])), static_cast<float>(std::atof(args[2])), static_cast<float>(std::atof(args[3])));
}
CON_COMMAND(sar_tas_set_prop, "sar_tas_set_prop <prop_name> : Sets value for sar_hud_player_info.\n")
{
    if (args.ArgC() < 2) {
        return console->Print("sar_tas_set_prop <prop_name> : Sets value for sar_hud_player_info.\n");
    }

    auto offset = 0;
    offsetFinder->ClientSide(tasTools->className, args[1], &offset);

    if (!offset) {
        console->Print("Unknown prop of %s!\n", tasTools->className);
    } else {
        std::strncpy(tasTools->propName, args[1], sizeof(tasTools->propName));
        tasTools->propOffset = offset;

        if (Utils::StartsWith(tasTools->propName, "m_b")) {
            tasTools->propType = PropType::Boolean;
        } else if (Utils::StartsWith(tasTools->propName, "m_f")) {
            tasTools->propType = PropType::Float;
        } else if (Utils::StartsWith(tasTools->propName, "m_vec") || Utils::StartsWith(tasTools->propName, "m_ang") || Utils::StartsWith(tasTools->propName, "m_q")) {
            tasTools->propType = PropType::Vector;
        } else if (Utils::StartsWith(tasTools->propName, "m_h") || Utils::StartsWith(tasTools->propName, "m_p")) {
            tasTools->propType = PropType::Handle;
        } else if (Utils::StartsWith(tasTools->propName, "m_sz") || Utils::StartsWith(tasTools->propName, "m_isz")) {
            tasTools->propType = PropType::String;
        } else if (Utils::StartsWith(tasTools->propName, "m_ch")) {
            tasTools->propType = PropType::Char;
        } else {
            tasTools->propType = PropType::Integer;
        }
    }

    console->Print("Current prop: %s::%s\n", tasTools->className, tasTools->propName);
}
CON_COMMAND(sar_tas_addang, "sar_tas_addang <x> <y> [z] : Adds {x, y, z} degrees to {x, y, z} view axis.\n")
{
    if (!sv_cheats.GetBool()) {
        return console->Print("Cannot use sar_tas_addang without sv_cheats set to 1.\n");
    }

    if (args.ArgC() < 3) {
        return console->Print("Missing arguments : sar_tas_addang <x> <y> [z].\n");
    }

    auto angles = engine->GetAngles();

    angles.x += static_cast<float>(std::atof(args[1]));
    angles.y += static_cast<float>(std::atof(args[2])); // Player orientation

    if (args.ArgC() == 4)
        angles.z += static_cast<float>(std::atof(args[3]));

    engine->SetAngles(angles);
}
CON_COMMAND(sar_tas_setang, "sar_tas_setang <x> <y> [z] : Sets {x, y, z} degrees to view axis.\n")
{
    if (!sv_cheats.GetBool()) {
        return console->Print("Cannot use sar_tas_setang without sv_cheats set to 1.\n");
    }

    if (args.ArgC() < 3) {
        return console->Print("Missing arguments : sar_tas_setang <x> <y> [z].\n");
    }

    // Fix the bug when z is not set
    if (args.ArgC() == 3) {
        engine->SetAngles(QAngle{ static_cast<float>(std::atof(args[1])), static_cast<float>(std::atof(args[2])), 0.0f });
    } else {
        engine->SetAngles(QAngle{ static_cast<float>(std::atof(args[1])), static_cast<float>(std::atof(args[2])), static_cast<float>(std::atof(args[3])) });
    }
}
CON_COMMAND(sar_tas_strafe, "sar_tas_strafe <direction> [vectorial] [strafing type].\n"
                            "Strafe while <direction> is -1 or 1. Set <vectorial> to use vectorial strafing.\n"
                            "<strafing type> : 0 - normal strafing; 1 - oscillating; 2 - turning\n")
{
    if (!sv_cheats.GetBool()) {
        return console->Print("Cannot use sar_tas_strafe without sv_cheats set to 1.\n");
    }

    if (args.ArgC() < 2) {
        return console->Print("Missing arguments : sar_tas_strafe <direction> [vectorial] [strafing type].\n");
    }

    int direction = std::atoi(args[1]);
    if (direction == 0) {
        tasTools->wantToStrafe = 0;
        tasTools->isVectorial = 0;
        tasTools->strafingDirection = 0;
        tasTools->strafeType = 0;
    } else {
        tasTools->wantToStrafe = 1;
        tasTools->strafingDirection = (direction > 0) ? 1 : -1;
        tasTools->isVectorial = std::atoi(args[2]);
        tasTools->strafeType = std::atoi(args[3]);
    }
}
