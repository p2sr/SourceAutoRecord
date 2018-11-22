#include "TasTools.hpp"

#include <cmath>
#include <cstring>

#include "Features/Session.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Utils/Math.hpp"
#include "Utils/SDK.hpp"
#include <Offsets.hpp>

TasTools* tasTools;

TasTools::TasTools()
    : m_previous_speed(Vector{ 0, 0, 0 })
    , m_last_tick(0)
    , m_acceleration(Vector{ 0, 0, 0 })
    , move(0)
{
    this->hasLoaded = true;
    strcpy(this->m_offset_name, "m_InAirState");
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
int TasTools::GetOffset()
{
    int anyOffset = (m_player) ? *reinterpret_cast<int*>((uintptr_t)m_player + this->m_offset) : -1;
    return anyOffset;
}
Vector TasTools::GetVelocityAngles()
{
    Vector velocity_angles = client->GetLocalVelocity();
    if (velocity_angles.Length() == 0)
        return Vector{ 0, 0, 0 };

    Math::VectorNormalize(velocity_angles);

    float yaw = atan2f(velocity_angles.y, velocity_angles.x);
    float pitch = atan2f(velocity_angles.z, sqrtf(velocity_angles.y * velocity_angles.y + velocity_angles.x * velocity_angles.x));

    return Vector{ RAD2DEG(yaw), RAD2DEG(pitch), 0 };
}
Vector TasTools::GetAcceleration()
{
    int current_tick = engine->GetSessionTick();
    Vector current_speed = client->GetLocalVelocity();
    if (current_tick != m_last_tick) { //Every frames
        m_acceleration.z = current_speed.Length2D() - m_previous_speed.Length2D(); //z used to represent the combined x/y acceleration axis value
        m_acceleration.x = abs(current_speed.x) - abs(m_previous_speed.x);
        m_acceleration.y = abs(current_speed.y) - abs(m_previous_speed.y);
        m_previous_speed = current_speed;
        m_last_tick = current_tick;
    }
    return m_acceleration;
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

CON_COMMAND(sar_tas_aim_at_point, "sar_tas_aim_at_point <x> <y> <z> : Aim at the point {x, y, z} specified.\n")
{
    if (!sv_cheats.GetBool()) {
        return console->Print("Cannot use sar_tas_aim_at_point without sv_cheats set to 1.\n");
    }

    if (args.ArgC() != 4) {
        return console->Print("sar_tas_aim_at_point <x> <y> <z> : Aim at the point {x, y, z} specified.\n");
    }

    tasTools->AimAtPoint(static_cast<float>(std::atof(args[1])), static_cast<float>(std::atof(args[2])), static_cast<float>(std::atof(args[3])));
}
CON_COMMAND(sar_get_offset, "sar_get_offset <Offset> : return the value of the offset.\n")
{
    if (args.ArgC() < 2) {
        console->Print("sar_get_offset <Offset> : return the value of the offset.\n");
        return;
    }
    std::strcpy(tasTools->m_offset_name, args[1]);
    tasTools->m_player = client->GetPlayer();
    client->GetOffset("CPortal_Player", args[1], tasTools->m_offset);
}
CON_COMMAND(sar_tas_addang, "sar_tas_addang <x> <y> [z] : add {x, y, z} degrees to {x, y, z} view axis.\n")
{
    if (!sv_cheats.GetBool()) {
        console->Print("Cannot use sar_tas_addang without sv_cheats sets to 1.\n");
        return;
    }
    if (args.ArgC() < 3) {
        console->Print("Missing arguments : sar_tas_addang <x> <y> [z].\n");
        return;
    }

    QAngle angles = engine->GetAngles();
    angles.x += static_cast<float>(std::atof(args[1]));
    angles.y += static_cast<float>(std::atof(args[2])); // Player orientation
    if (args.ArgC() == 4)
        angles.z += static_cast<float>(std::atof(args[3]));

    engine->SetAngles(angles);
}
CON_COMMAND(sar_tas_setang, "sar_tas_setang <x> <y> [z] : set {x, y, z} degres to view axis.\n")
{
    if (!sv_cheats.GetBool()) {
        console->Print("Cannot use sar_tas_setang without sv_cheats sets to 1.\n");
        return;
    }
    if (args.ArgC() < 3) {
        console->Print("Missing arguments : sar_tas_setang <x> <y> [z].\n");
        return;
    }

    QAngle angle = { static_cast<float>(std::atof(args[1])), static_cast<float>(std::atof(args[2])), static_cast<float>(std::atof(args[3])) };
    engine->SetAngles(angle);
}
CON_COMMAND(sar_groundstrafe, "sar_groundstrafe <opposite> [2D]\n")
{
	if (!sv_cheats.GetBool()){
        console->Print("Cannot use sar_groundstrafe without sv_cheats sets to 1.\n");
		return;
	}
	if (args.ArgC() < 2) {
		console->Print("Missing arguments : sar_groundstrafe <opposite> [2D]\n");
		return;
	}
	tasTools->Strafe(std::atoi(args[1]), 1, std::atoi(args[2]));
}
CON_COMMAND(sar_airstrafe, "sar_strafe <opposite> [2D]\n")
{
    if (!sv_cheats.GetBool()) {
        console->Print("Cannot use sar_strafe without sv_cheats sets to 1.\n");
        return;
    }
    if (args.ArgC() < 2) {
        console->Print("Missing arguments : sar_strafe <opposite> [2D]\n");
        return;
    }
	tasTools->Strafe(std::atoi(args[1]), 0, std::atoi(args[2]));
}