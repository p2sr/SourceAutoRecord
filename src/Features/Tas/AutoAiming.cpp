#include "AutoAiming.hpp"

#include <cmath>

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Utils/SDK.hpp"

AutoAiming* autoAiming;

AutoAiming::AutoAiming()
{
    this->hasLoaded = true;
}
void AutoAiming::AimAtPoint(float x, float y, float z)
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

    autoAiming->AimAtPoint(atof(args[1]), atof(args[2]), atof(args[3]));
}
