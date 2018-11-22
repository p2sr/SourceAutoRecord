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
    , acceleration({0, 0, 0})
    , prevVelocity({ 0, 0, 0 })
    , prevTick(0)
    , move(0)
{
    if (sar.game->version & (SourceGame_Portal | SourceGame_Portal2)) {
        std::strncpy(this->className, "CPortal_Player", sizeof(this->className));
    } else if (sar.game->version & SourceGame_HalfLife2) {
        std::strncpy(this->className, "CHL2_Player", sizeof(this->className));
    } else {
        std::strncpy(this->className, "CBasePlayer", sizeof(this->className));
    }

    client->GetOffset(this->className, this-> propName, this->propOffset);

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
void TasTools::Strafe(int opposite, int grounded, int in_2D)
{
    float tau = 1 / host_framerate.GetFloat(); //A time for one frame to pass, don't know if it's an actual value you can get smh

    //Creating lambda(v) - velocity after ground friction
    float player_friction = 1;
    float friction = sv_friction.GetFloat() * player_friction * 1;
    Vector velocity = client->GetLocalVelocity();
    Vector lambda = velocity;
    
	//Get the grounded status
	/*int is_inAir;
    auto player = client->GetPlayer();
    client->GetOffset("CPortal_Player", "m_InAirState", is_inAir);
    int grounded = (player) ? !(*reinterpret_cast<int*>((uintptr_t)player + is_inAir)) : -1;*/

    if (grounded) {
        if ((in_2D) ? velocity.Length() : velocity.Length2D() >= sv_stopspeed.GetFloat()) {
            lambda = lambda * (1.0 - tau * friction);
        } else if ((in_2D) ? velocity.Length() : velocity.Length2D() >= std::fmax(0.1, tau * sv_stopspeed.GetFloat() * friction)) {
            Math::VectorNormalize(velocity);
            lambda = lambda + (velocity * (tau * sv_stopspeed.GetFloat() * friction)) * -1; //lambda -= v * tau * stop * friction
        } else {
            lambda = lambda * 0;
        }
    }

    //Getting M
    int jump_state = (this->move & IN_JUMP) ? 1 : 0; 
	int forward_keystate = (this->move & IN_FORWARD) ? 1 : 0;
	int backward_keystate = (this->move & IN_BACK) ? 1 : 0;
	int moveright_keystate = (this->move & IN_MOVELEFT) ? 1 : 0;
	int moveleft_keystate = (this->move & IN_MOVERIGHT) ? 1 : 0;

    float F = forward_keystate - backward_keystate;
    float S = moveright_keystate - moveleft_keystate;

    //float F = 1;
    //float S = 0;

    float stateLen = sqrt(F * F + S * S);
    float forwardMove = cl_forwardspeed.GetFloat() * F / stateLen;
    float sideMove = cl_sidespeed.GetFloat() * S / stateLen;
    float M = std::fminf(sv_maxspeed.GetFloat(), sqrt(forwardMove * forwardMove + sideMove * sideMove));

    //Getting other stuff
    float A = (grounded) ? sv_accelerate.GetFloat() : sv_airaccelerate.GetFloat();
    float L = (grounded) ? M : std::fmin(60, M);

    //Getting the most optimal angle
    float cosTheta = (L - player_friction * tau * M * A) / (in_2D) ? lambda.Length2D() : lambda.Length();
    if (cosTheta < 0)
        cosTheta = M_PI_F / 2;
    if (cosTheta > 1)
        cosTheta = 0;

    float theta = acosf(cosTheta) * (opposite ? -1 : 1);

	console->Print("Friction: %f, velocity.length: %f, lambda.lenght: %f, M: %f, A: %f, L: %f, cosTheta: %f, theta: %f\n\n",
        friction, (in_2D) ? velocity.Length() : velocity.Length2D(), (in_2D) ? lambda.Length2D() : lambda.Length(), M, A, L, cosTheta, theta);

    engine->SetAngles({ 0, this->GetVelocityAngles().x + RAD2DEG(theta), 0 });
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
CON_COMMAND(sar_groundstrafe, "sar_groundstrafe <opposite> [2D]\n")
{
	if (!sv_cheats.GetBool())
		return console->Print("Cannot use sar_groundstrafe without sv_cheats sets to 1.\n");
  
	if (args.ArgC() < 2)
		return console->Print("Missing arguments : sar_groundstrafe <opposite> [2D]\n");
  
	tasTools->Strafe(std::atoi(args[1]), 1, std::atoi(args[2]));
}
CON_COMMAND(sar_airstrafe, "sar_strafe <opposite> [2D]\n")
{
    if (!sv_cheats.GetBool())
        return console->Print("Cannot use sar_strafe without sv_cheats sets to 1.\n");
  
    if (args.ArgC() < 2)
        return console->Print("Missing arguments : sar_strafe <opposite> [2D]\n");
      
	tasTools->Strafe(std::atoi(args[1]), 0, std::atoi(args[2]));
}