#include "TasTools.hpp"

#include <cmath>
#include <cstring>

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Utils/Math.hpp"
#include "Utils/SDK.hpp"

#include "SAR.hpp"

TasTools* tasTools;

TasTools::TasTools()
    : propName("m_InAirState")
    , propType(PropType::Integer)
    , acceleration({ 0, 0, 0 })
    , prevVelocity({ 0, 0, 0 })
    , prevTick(0)
    , buttonBits(0)
    , m_backward_was_pressed(0)
    , m_forward_was_pressed(0)
    , m_moveleft_was_pressed(0)
    , m_moveright_was_pressed(0)
    , m_jump_was_pressed(0)
    , want_to_strafe(0)
    , strafing_direction(0)
    , moves({ 0 })
{
    if (sar.game->version & (SourceGame_Portal | SourceGame_Portal2)) {
        std::strncpy(this->className, "CPortal_Player", sizeof(this->className));
    } else if (sar.game->version & SourceGame_HalfLife2) {
        std::strncpy(this->className, "CHL2_Player", sizeof(this->className));
    } else {
        std::strncpy(this->className, "CBasePlayer", sizeof(this->className));
    }

    client->GetOffset(this->className, this->propName, this->propOffset);

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
void TasTools::SetButtonBits(int buttonBits)
{
    this->buttonBits = buttonBits;
}
void TasTools::SetMoveButtonsState(float forward, float backward, float moveright, float moveleft, float jump)
{
    this->m_forward_was_pressed = forward;
    this->m_backward_was_pressed = backward;
    this->m_moveright_was_pressed = moveright;
    this->m_moveleft_was_pressed = moveleft;
    this->m_jump_was_pressed = jump;
}
MoveInputs TasTools::GetMoveInputs()
{
    float forward_keystate = (this->buttonBits & IN_FORWARD) ? 1 : 0;
    float backward_keystate = (this->buttonBits & IN_BACK) ? 1 : 0;
    float moveright_keystate = (this->buttonBits & IN_MOVELEFT) ? 1 : 0;
    float moveleft_keystate = (this->buttonBits & IN_MOVERIGHT) ? 1 : 0;
    float jump_keystate = (this->buttonBits & IN_JUMP) ? 1 : 0;

    //Managing 0.5 inputs
    if (!forward_keystate) {
        this->m_forward_was_pressed = 0;
    } else if (forward_keystate && !this->m_forward_was_pressed) {
        this->m_forward_was_pressed = 1;
        forward_keystate = 0.5;
    }
    if (!backward_keystate) {
        this->m_backward_was_pressed = 0;
    } else if (backward_keystate && !this->m_backward_was_pressed) {
        this->m_backward_was_pressed = 1;
        backward_keystate = 0.5;
    }
    if (!moveright_keystate) {
        this->m_moveright_was_pressed = 0;
    } else if (moveright_keystate && !this->m_moveright_was_pressed) {
        this->m_moveright_was_pressed = 1;
        moveright_keystate = 0.5;
    }
    if (!moveleft_keystate) {
        this->m_moveleft_was_pressed = 0;
    } else if (moveleft_keystate && !this->m_moveleft_was_pressed) {
        this->m_moveleft_was_pressed = 1;
        moveleft_keystate = 0.5;
    }
    if (!jump_keystate) {
        this->m_jump_was_pressed = 0;
    } else if (jump_keystate && !this->m_jump_was_pressed) {
        this->m_jump_was_pressed = 1;
        jump_keystate = 0.5;
    }

    return MoveInputs{ forward_keystate, backward_keystate, moveleft_keystate, moveright_keystate, jump_keystate };
}
//Returns an absolute angle for perfect wish direction vector
float TasTools::GetStrafeAngle(CMoveData* pmove, int direction)
{
    float tau = 1 / host_framerate.GetFloat(); //A time for one frame to pass

    //For getting the offsets needed
    auto player = client->GetPlayer();

    //Getting the player inputs
    MoveInputs inputs = this->GetMoveInputs();

    //Getting player's friction
    int player_friction_offset = 0;
    client->GetOffset("CPortal_Player", "m_flFriction", player_friction_offset);
    float player_friction = (*reinterpret_cast<float*>((uintptr_t)player + player_friction_offset));
    float friction = sv_friction.GetFloat() * player_friction * 1;

    //Creating lambda(v) - velocity after ground friction
    Vector velocity = client->GetLocalVelocity();
    Vector lambda = velocity;

    //Getting the grounded status
    /*int is_inAir;
    client->GetOffset("CPortal_Player", "m_InAirState", is_inAir);
    int grounded = (inputs.jump == 0.5 || ((player) ? !(*reinterpret_cast<int*>((uintptr_t)player + is_inAir)) : 0));*/

    bool grounded = pmove->m_vecVelocity.z == 0;
    if (grounded) {
        if (velocity.Length2D() >= sv_stopspeed.GetFloat()) {
            lambda = lambda * (1.0 - tau * friction);
        } else if (velocity.Length2D() >= std::fmax(0.1, tau * sv_stopspeed.GetFloat() * friction)) {
            Vector normalized_velocity = velocity;
            Math::VectorNormalize(normalized_velocity);
            lambda = lambda + (normalized_velocity * (tau * sv_stopspeed.GetFloat() * friction)) * -1; //lambda -= v * tau * stop * friction
        } else {
            lambda = lambda * 0;
        }
    }

    //Getting M
    float F = pmove->m_flForwardMove / cl_forwardspeed.GetFloat();
    float S = pmove->m_flSideMove / cl_sidespeed.GetFloat();

    float stateLen = sqrt(F * F + S * S);
    float forwardMove = 0;
    float sideMove = 0;
    if (pmove->m_flForwardMove != 0)
        forwardMove = pmove->m_flForwardMove / stateLen;
    if (pmove->m_flSideMove != 0)
        sideMove = pmove->m_flSideMove / stateLen;

    float M = std::fminf(sv_maxspeed.GetFloat(), sqrt(forwardMove * forwardMove + sideMove * sideMove));

    //Getting other stuff
    float A = (grounded) ? sv_accelerate.GetFloat() : sv_airaccelerate.GetFloat() / 2;
    float L = (grounded) ? M : std::fmin(60, M);

    //Getting the most optimal angle
    float cosTheta = (L - player_friction * tau * M * A) / lambda.Length2D();
    if (cosTheta < 0)
        cosTheta = M_PI_F / 2;
    if (cosTheta > 1)
        cosTheta = 0;

    float theta = acosf(cosTheta) * ((direction > 0) ? -1 : 1);
    float lookangle = std::atan2f(sideMove, forwardMove);

    return this->GetVelocityAngles().x + RAD2DEG(theta);
}
//Angle set based on current wishdir
void TasTools::Strafe(CMoveData* pmove)
{
    float angle = this->GetStrafeAngle(pmove, this->strafing_direction);
    float lookangle = RAD2DEG(std::atan2f(pmove->m_flSideMove, pmove->m_flForwardMove));

    QAngle newAngle = { 0, angle + lookangle, 0 };
    pmove->m_vecViewAngles = newAngle;
    pmove->m_vecAbsViewAngles = newAngle;

	console->Print("LookAngle: %f\n\n", lookangle);

    engine->SetAngles(newAngle);
}
//whishdir set based on current angle ("controller" inputs)
void TasTools::VectorialStrafe(CMoveData* pmove)
{

	console->Print("vecAbsViewAngle: x %f, y: %f, z: %f\n\n", pmove->m_vecAbsViewAngles.x, pmove->m_vecAbsViewAngles.y, pmove->m_vecAbsViewAngles.z);

    float angle = this->GetStrafeAngle(pmove, this->strafing_direction);
    angle = DEG2RAD(angle - pmove->m_vecAbsViewAngles.y);

    pmove->m_flForwardMove = cosf(angle) * cl_forwardspeed.GetFloat();
    pmove->m_flSideMove = -sinf(angle) * cl_sidespeed.GetFloat();

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
    client->GetOffset(tasTools->className, args[1], offset);

    if (!offset) {
        console->Print("Unknown prop of %s!\n", tasTools->className);
    } else {
        std::strncpy(tasTools->propName, args[1], sizeof(tasTools->propName));
        tasTools->propOffset = offset;

        if (std::strstr(tasTools->propName, "m_b") == tasTools->propName) {
            tasTools->propType = PropType::Boolean;
        } else if (std::strstr(tasTools->propName, "m_f") == tasTools->propName) {
            tasTools->propType = PropType::Float;
        } else if (std::strstr(tasTools->propName, "m_vec") == tasTools->propName
            || std::strstr(tasTools->propName, "m_ang") == tasTools->propName
            || std::strstr(tasTools->propName, "m_q") == tasTools->propName) {
            tasTools->propType = PropType::Vector;
        } else if (std::strstr(tasTools->propName, "m_h") == tasTools->propName
            || std::strstr(tasTools->propName, "m_p") == tasTools->propName) {
            tasTools->propType = PropType::Handle;
        } else if (std::strstr(tasTools->propName, "m_sz") == tasTools->propName
            || std::strstr(tasTools->propName, "m_isz") == tasTools->propName) {
            tasTools->propType = PropType::String;
        } else if (std::strstr(tasTools->propName, "m_ch") == tasTools->propName) {
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
        return console->Print("Cannot use sar_tas_addang without sv_cheats sets to 1.\n");
    }

    if (args.ArgC() < 3) {
        return console->Print("Missing arguments : sar_tas_addang <x> <y> [z].\n");
    }

    auto angles = engine->GetAngles();

    angles.x += static_cast<float>(std::atof(args[1]));
    angles.y += static_cast<float>(std::atof(args[2])); // Player orientation

    if (args.ArgC() == 4) {
        angles.z += static_cast<float>(std::atof(args[3]));
    }

    engine->SetAngles(angles);
}
CON_COMMAND(sar_tas_setang, "sar_tas_setang <x> <y> [z] : Sets {x, y, z} degres to view axis.\n")
{
    if (!sv_cheats.GetBool()) {
        return console->Print("Cannot use sar_tas_setang without sv_cheats sets to 1.\n");
    }

    if (args.ArgC() < 3) {
        return console->Print("Missing arguments : sar_tas_setang <x> <y> [z].\n");
    }

    QAngle angle = { static_cast<float>(std::atof(args[1])), static_cast<float>(std::atof(args[2])), static_cast<float>(std::atof(args[3])) };
    engine->SetAngles(angle);
}
CON_COMMAND(sar_tas_strafe, "sar_tas_strafe <direction> <vectorial>\n")
{
    if (!sv_cheats.GetBool())
        return console->Print("Cannot use sar_tas_strafe without sv_cheats sets to 1.\n");

    if (args.ArgC() < 3)
        return console->Print("Missing arguments : sar_tas_strafe <direction> <vectorial>\n");

    int direction = std::atoi(args[1]);
    if (direction == 0) {
        tasTools->want_to_strafe = 0;
        tasTools->strafeMode = 0;
        tasTools->strafing_direction = 0;
    } else {
        tasTools->want_to_strafe = 1;
        tasTools->strafeMode = std::atoi(args[2]);
        tasTools->strafing_direction = (direction > 0) ? 1 : -1;
    }

    /*tasTools->strafing_direction = std::atoi(args[1]);
    tasTools->want_to_strafe = std::atoi(args[2]);
    tasTools->Strafe();*/
}