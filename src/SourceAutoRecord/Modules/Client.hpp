#pragma once
#include "vmthook/vmthook.h"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/TAS.hpp"

#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

namespace Client
{
	using _GetLocalPlayer = void*(__cdecl*)(int unk);
	using _HudUpdate = int(__cdecl*)(void* thisptr, unsigned int a2);

	std::unique_ptr<VMTHook> clientdll;

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

	namespace Original
	{
		_HudUpdate HudUpdate;
	}

	namespace Detour
	{
		int __cdecl HudUpdate(void* thisptr, unsigned int a2)
		{
			if (TAS::IsRunning)
			{
				if (TAS::FrameDelay - 1 < 0)
				{
					for (auto tas = TAS::Frames.begin(); tas != TAS::Frames.end(); )
					{
						tas->FramesLeft--;
						if (tas->FramesLeft <= 0)
						{
							Console::DevMsg("TAS: %s\n", tas->Command.c_str());
							Engine::ExecuteCommand(tas->Command.c_str());
							tas = TAS::Frames.erase(tas);
						}
						else {
							tas++;
						}
					}
				}
				else {
					TAS::FrameDelay--;
				}
			}
			return Original::HudUpdate(thisptr, a2);
		}
	}

	void Hook()
	{
		if (Interfaces::IBaseClientDLL) {
			clientdll = std::make_unique<VMTHook>(Interfaces::IBaseClientDLL);
			clientdll->HookFunction((void*)Detour::HudUpdate, Offsets::HudUpdate);
			Original::HudUpdate = clientdll->GetOriginalFunction<_HudUpdate>(Offsets::HudUpdate);
		}

		auto client = MODULEINFO();
		if (GetModuleInformation("client.so", &client)) {
			//GetLocalPlayer = reinterpret_cast<_GetLocalPlayer>(0xB298F0 + client.lpBaseOfDll);
			GetLocalPlayer = reinterpret_cast<_GetLocalPlayer>(0x9C1890 + client.lpBaseOfDll);
			MainViewOrigin = *reinterpret_cast<Vector**>(0xDB6DB0 + client.lpBaseOfDll + Offsets::MainViewOrigin);
			MainViewAngles = *reinterpret_cast<QAngle**>(0xDB6DB0 + client.lpBaseOfDll + Offsets::MainViewAngles);
		}
		else {
			Console::Warning("Failed to get module info for client.so!\n");
		}
	}
}