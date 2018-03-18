#pragma once
#include "Offsets.hpp"
#include "Utils.hpp"

namespace Client
{
	using _GetLocalPlayer = void*(__cdecl*)(int unk);
	_GetLocalPlayer GetLocalPlayer;

	Vector* MainViewOrigin;
	QAngle* MainViewAngles;

	Vector GetAbsOrigin()
	{
		auto player = GetLocalPlayer(-1);
		return (player) ? *(Vector*)((uintptr_t)player + Offsets::GetAbsOrigin) : Vector();
	}
	QAngle GetAbsAngles()
	{
		auto player = GetLocalPlayer(-1);
		return (player) ? *(QAngle*)((uintptr_t)player + Offsets::GetAbsAngles) : QAngle();
	}
	Vector GetLocalVelocity()
	{
		auto player = GetLocalPlayer(-1);
		return (player) ? *(Vector*)((uintptr_t)player + Offsets::GetLocalVelocity) : Vector();
	}

	void Hook()
	{
		auto client = MODULEINFO();
		if (GetModuleInformation("client.so", &client)) {
			GetLocalPlayer = reinterpret_cast<_GetLocalPlayer>(0xB298F0 + client.lpBaseOfDll);
			MainViewOrigin = *reinterpret_cast<Vector**>(0xDB6DB0 + client.lpBaseOfDll + Offsets::MainViewOrigin);
			MainViewAngles = *reinterpret_cast<QAngle**>(0xDB6DB0 + client.lpBaseOfDll + Offsets::MainViewAngles);
		}
		else {
			Console::Warning("Failed to get module info for client.so!\n");
		}
	}
}