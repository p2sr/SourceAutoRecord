#include "TasTools.hpp"

#include <cmath>

#include "Features/Session.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Utils/Math.hpp"
#include "Utils/SDK.hpp"
#include <Offsets.hpp>

TasTools* tasTools;

TasTools::TasTools()
{
    this->hasLoaded = true;
    this->m_previous_speed = Vector{ 0, 0, 0 };
    this->m_last_tick = (session->isInSession) ? engine->GetSessionTick() : 0;
    this->m_acceleration = Vector{ 0, 0, 0 };

    strcpy(this->m_offset_name, "m_InAirState");
    strcpy(this->m_class_name, "CPortal_Player");
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

// Commands

CON_COMMAND(sar_tas_aim_at_point, "sar_tas_aim_at_point <x> <y> <z> : Aim at the point {x, y, z} specified.\n")
{
    if (!sv_cheats.GetBool()) {
        return console->Print("Cannot use sar_tas_aim_at_point without sv_cheats set to 1.\n");
    }

    if (args.ArgC() != 4) {
        return console->Print("sar_tas_aim_at_point <x> <y> <z> : Aim at the point {x, y, z} specified.\n");
    }

    tasTools->AimAtPoint(static_cast<float>(atof(args[1])), static_cast<float>(atof(args[2])), static_cast<float>(atof(args[3])));
}


int TasTools::GetOffset()
{
    client->GetOffset(this->m_class_name, this->m_offset_name, Offsets::anyOffset);
    auto player = client->GetPlayer();
    int anyOffset = (player) ? *reinterpret_cast<int*>((uintptr_t)player + Offsets::anyOffset) : -1;

    return (anyOffset);

}


CON_COMMAND(sar_get_offset, "sar_get_offset <Class_name> <Offset> : return the value of the offset.\n")
{
	if (args.ArgC() < 3) {
        console->Print("sar_get_offset <Class_name> <Offset> : return the value of the offset.\n");
        return;
	}

	std::strcpy(tasTools->m_class_name, args[1]);
    std::strcpy(tasTools->m_offset_name, args[2]);

}


CON_COMMAND(sar_tas_addang, "sar_addang <x> <y> [z] : add {x, y, z} degrees to {x, y, z} view axis.\n")
{

    if (args.ArgC() < 3) {
        console->Print("Missing arguments : sar_addang <x> <y> [z].\n");
        return;
    }

    QAngle angles = engine->GetAngles();
    angles.x += static_cast<float>(atof(args[1]));
    angles.y += static_cast<float>(atof(args[2])); // Player orientation
    if (args.ArgC() == 4)
        angles.z += static_cast<float>(atof(args[3]));

    engine->SetAngles(angles);
}

CON_COMMAND(sar_tas_setang, "sar_tas_setang <x> <y> [z] : set {x, y, z} degres to view axis.\n")
{
    QAngle angle = { static_cast<float>(atof(args[1])), static_cast<float>(atof(args[2])), static_cast<float>(atof(args[3])) };
    engine->SetAngles(angle);
}

Vector TasTools::GetVelocityAngles()
{
    Vector velocity_angles = client->GetLocalVelocity();
    if (velocity_angles.Length() == 0)
        return (Vector{ 0, 0, 0 });

    Math::VectorNormalize(velocity_angles);

    float yaw = atan2f(velocity_angles.y, velocity_angles.x);
    float pitch = atan2f(velocity_angles.z, sqrtf(velocity_angles.y * velocity_angles.y + velocity_angles.x * velocity_angles.x));

    return (Vector{ RAD2DEG(yaw), RAD2DEG(pitch), 0 });
}

Vector& TasTools::GetAcceleration()
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

//TODO : Finish this function
/*CON_COMMAND(sar_tas_strafe, "sar_tas_strafe : do an optimized strafe.\n")
{
    QAngle angle = {0, (-41.4423 + log(std::fmax(client->GetLocalVelocity().Length2D() - 144.31, 0)) * 19.919) + tasTools->GetVelocityAngles().x, 0};
	engine->SetAngles(angle);
}*/


