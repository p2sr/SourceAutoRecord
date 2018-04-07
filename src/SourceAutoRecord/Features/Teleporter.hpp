#pragma once
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"

#include "Utils.hpp"

namespace Teleporter
{
	bool IsSet;

	Vector Origin;
	QAngle Angles;

	void Save()
	{
		IsSet = true;
		Origin = Client::GetAbsOrigin();
		Angles = Engine::GetAngles();
		Console::Print("Saved location: %.3f %.3f %.3f\n", Origin.x, Origin.y, Origin.z);
	}
	void Teleport()
	{
		Engine::SetAngles(Angles);
		char setpos[64];
		snprintf(setpos, sizeof(setpos), "setpos %f %f %f", Origin.x, Origin.y, Origin.z);
		Engine::ExecuteCommand(setpos);
	}
}