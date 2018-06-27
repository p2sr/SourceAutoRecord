#pragma once
#include "vmthook/vmthook.h"

#include "Console.hpp"
#include "Engine.hpp"

#include "Features/TAS.hpp"

#include "Game.hpp"
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

namespace Client
{
    using _HudUpdate = int(__cdecl*)(void* thisptr, unsigned int a2);
    using _CreateMove = int(__cdecl*)(void* thisptr, float flInputSampleTime, CUserCmd* cmd);
    using _GetClientEntity = void*(__cdecl*)(void* thisptr, int entnum);
    using _KeyDown = int(__cdecl*)(void* b, const char* c);
    using _KeyUp = int(__cdecl*)(void* b, const char* c);

    std::unique_ptr<VMTHook> clientdll;
    std::unique_ptr<VMTHook> s_EntityList;
    std::unique_ptr<VMTHook> g_pClientMode;
    std::unique_ptr<VMTHook> g_Input;

    _GetClientEntity GetClientEntity;

    void* in_jump;
    _KeyDown KeyDown;
    _KeyUp KeyUp;

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
    int GetFlags()
    {
        auto player = GetPlayer();
        return (player) ? *(int*)((uintptr_t)player + Offsets::GetFlags) : 0;
    }

    namespace Original
    {
        _HudUpdate HudUpdate;
        _CreateMove CreateMove;
    }

    namespace Detour
    {
        int __cdecl HudUpdate(void* thisptr, unsigned int a2)
        {
            if (TAS::IsRunning) {
                for (auto tas = TAS::Frames.begin(); tas != TAS::Frames.end();) {
                    tas->FramesLeft--;
                    if (tas->FramesLeft <= 0) {
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
        int __cdecl CreateMove(void* thisptr, float flInputSampleTime, CUserCmd* cmd)
        {
            return Original::CreateMove(thisptr, flInputSampleTime, cmd);
        }
    }

    void Hook()
    {
        if (Interfaces::IBaseClientDLL) {
            clientdll = std::make_unique<VMTHook>(Interfaces::IBaseClientDLL);
            clientdll->HookFunction((void*)Detour::HudUpdate, Offsets::HudUpdate);
            Original::HudUpdate = clientdll->GetOriginalFunction<_HudUpdate>(Offsets::HudUpdate);

            /* // Before HudUpdate in VMT
            auto HudProcessInput = clientdll->GetOriginalFunction<uintptr_t>(Offsets::HudUpdate - 1);
            typedef void*(*_GetClientMode)();
            auto GetClientMode = reinterpret_cast<_GetClientMode>(GetAbsoluteAddress(HudProcessInput + 12));
            g_pClientMode = std::make_unique<VMTHook>(GetClientMode());

            g_pClientMode->HookFunction((void*)Detour::CreateMove, Offsets::CreateMove);
            Original::CreateMove = g_pClientMode->GetOriginalFunction<_CreateMove>(Offsets::CreateMove); */

            if (Game::Version == Game::TheStanleyParable) {
                auto IN_ActivateMous = clientdll->GetOriginalFunction<uintptr_t>(Offsets::IN_ActivateMous);
                auto input = **reinterpret_cast<void***>(IN_ActivateMous + 1);
                g_Input = std::make_unique<VMTHook>(input);;

                auto GetButtonBits = g_Input->GetOriginalFunction<uintptr_t>(Offsets::GetButtonBits);
                in_jump = *reinterpret_cast<void**>((uintptr_t)GetButtonBits + Offsets::in_jump);

                auto JoyStickApplyMovement = g_Input->GetOriginalFunction<uintptr_t>(Offsets::JoyStickApplyMovement);
                KeyDown = reinterpret_cast<_KeyDown>(GetAbsoluteAddress(JoyStickApplyMovement + Offsets::KeyDown));
                KeyUp = reinterpret_cast<_KeyUp>(GetAbsoluteAddress(JoyStickApplyMovement + Offsets::KeyUp));
            }

            if (Interfaces::IClientEntityList) {
                s_EntityList = std::make_unique<VMTHook>(Interfaces::IClientEntityList);
                GetClientEntity = s_EntityList->GetOriginalFunction<_GetClientEntity>(Offsets::GetClientEntity);
            }
        }
    }
}