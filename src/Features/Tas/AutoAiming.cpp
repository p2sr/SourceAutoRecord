#include "Modules/Engine.hpp"
#include "Modules/Client.hpp"
#include "Modules/Server.hpp"

#include <string>
#include "Utils\Math.hpp"
#include <cmath>

#include "AutoAiming.hpp"

AutoAiming *autoAiming;

void AutoAiming::AimAtPoint(float tar_x, float tar_y, float tar_z)
{
	Vector pos = { client->GetAbsOrigin().y, client->GetAbsOrigin().x, client->GetAbsOrigin().z };
	Vector target = { tar_y, tar_x, tar_z - 64 }; //tar.z - 64 because of the camera height

	//Axis in the game, need to know it to fix up:
	//             : G - D ; Av - Ar ; H - B
	//Rotation axis:   x        z        y
	//Translation  :   y        x        z

	float xdis = target.x - pos.x;
	float ydis = target.z - pos.z;
	float zdis = target.y - pos.y;
	float xzdis = sqrtf(xdis * xdis + zdis * zdis);

	QAngle a = { RAD2DEG(-atan2f(ydis, xzdis)), RAD2DEG(-(atan2f(-xdis, zdis))), 0 };

	engine->SetAngles(a);
}



CON_COMMAND(sar_tas_aim_at_point, "sar_tas_aim_at_point <x> <y> <z> : Aim at the point {x, y, z} specified\n")
{
	if (sv_cheats.GetBool())
	{
		if (args.ArgC() != 4)
		{
			console->Print("sar_tas_aim_at_point <x> <y> <z> : Aim at the point {x, y, z} specified\n");
			return;
		}

		autoAiming->AimAtPoint(atof(args[1]), atof(args[2]), atof(args[3]));
	}
	else
	{
		console->Print("Cannot use sar_tas_aim_at_point without sv_cheats sets to 1\n");
	}
}
