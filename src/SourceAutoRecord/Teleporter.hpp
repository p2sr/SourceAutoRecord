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
		Angles = *Client::MainViewAngles;
		Console::Print("Saved location: %.3f %.3f %.3f\n", Origin.x, Origin.y, Origin.z);
	}
	std::string GetSetpos()
	{
		std::string setpos = "setpos "
			+ std::to_string(Origin.x) + " "
			+ std::to_string(Origin.y) + " "
			+ std::to_string(Origin.z);
		return setpos;
	}
	std::string GetSetang()
	{
		std::string setang = std::string("setang_exact ")
			+ std::to_string(Angles.x) + " "
			+ std::to_string(Angles.y) + " "
			+ std::to_string(Angles.z);
		return setang;
	}
}