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
	using _HudUpdate = int(__cdecl*)(void* thisptr, unsigned int a2);
	using _GetClientEntity = void*(__cdecl*)(void* thisptr, int entnum);

	std::unique_ptr<VMTHook> clientdll;
	std::unique_ptr<VMTHook> s_EntityList;

	_GetClientEntity GetClientEntity;

	void* GetPlayer()
	{
		return GetClientEntity(s_EntityList->GetThisPtr(), Engine::GetPlayerIndex());
	}
	
	Vector GetAbsOrigin()
	{
		auto player = GetPlayer();
		return (player) ? *(Vector*)((uintptr_t)player + Offsets::m_vecAbsOrigin) : Vector();
	}
	QAngle GetAbsAngles()
	{
		auto player = GetPlayer();
		return (player) ? *(QAngle*)((uintptr_t)player + Offsets::m_angAbsRotation) : QAngle();
	}
	Vector GetLocalVelocity()
	{
		auto player = GetPlayer();
		return (player) ? *(Vector*)((uintptr_t)player + Offsets::m_vecVelocity) : Vector();
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
				for (auto tas = TAS::Frames.begin(); tas != TAS::Frames.end();)
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

		if (Interfaces::IClientEntityList) {
			s_EntityList = std::make_unique<VMTHook>(Interfaces::IClientEntityList);
			GetClientEntity = s_EntityList->GetOriginalFunction<_GetClientEntity>(Offsets::GetClientEntity);
		}
	}
	void Unhook()
	{
		if (clientdll) {
			clientdll->UnhookFunction(Offsets::HudUpdate);
			clientdll->~VMTHook();
			clientdll.release();
			Original::HudUpdate = nullptr;
		}

		if (s_EntityList) {
			s_EntityList->~VMTHook();
			s_EntityList.release();
			GetClientEntity = nullptr;
		}
	}
}