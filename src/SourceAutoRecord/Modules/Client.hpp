#pragma once
#include "vmthook/vmthook.h"

#include "Console.hpp"
#include "Engine.hpp"

#include "Features/TAS.hpp"

#include "Commands.hpp"
#include "Game.hpp"
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

namespace Client {

using _HudUpdate = int(__thiscall*)(void* thisptr, unsigned int a2);
using _CreateMove = int(__thiscall*)(void* thisptr, float flInputSampleTime, CUserCmd* cmd);
using _GetClientEntity = void*(__thiscall*)(void* thisptr, int entnum);
using _KeyDown = int(__cdecl*)(void* b, const char* c);
using _KeyUp = int(__cdecl*)(void* b, const char* c);
using _GetName = const char*(__thiscall*)(void* thisptr);

std::unique_ptr<VMTHook> clientdll;
std::unique_ptr<VMTHook> s_EntityList;
std::unique_ptr<VMTHook> g_pClientMode;
std::unique_ptr<VMTHook> g_Input;
std::unique_ptr<VMTHook> g_HUDChallengeStats;

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

namespace Original {
    _HudUpdate HudUpdate;
    _CreateMove CreateMove;
    _GetName GetName;
}

namespace Detour {
    int __fastcall HudUpdate(void* thisptr, int edx, unsigned int a2)
    {
        if (TAS::IsRunning) {
            for (auto tas = TAS::Frames.begin(); tas != TAS::Frames.end();) {
                tas->FramesLeft--;
                if (tas->FramesLeft <= 0) {
                    Console::DevMsg("TAS: %s\n", tas->Command.c_str());
                    Engine::ExecuteCommand(tas->Command.c_str());
                    tas = TAS::Frames.erase(tas);
                } else {
                    tas++;
                }
            }
        }
        return Original::HudUpdate(thisptr, a2);
    }
    int __fastcall CreateMove(void* thisptr, int edx, float flInputSampleTime, CUserCmd* cmd)
    {
        return Original::CreateMove(thisptr, flInputSampleTime, cmd);
    }
    const char* __fastcall GetName(void* thisptr, int eax)
    {
        // Never allow CHud::FindElement to find this HUD
        if (sar_disable_challenge_stats_hud.GetBool())
            return "";

        return Original::GetName(thisptr);
    }
}

void Hook()
{
    if (Interfaces::IBaseClientDLL) {
        clientdll = std::make_unique<VMTHook>(Interfaces::IBaseClientDLL);
        clientdll->HookFunction((void*)Detour::HudUpdate, Offsets::HudUpdate);
        Original::HudUpdate = clientdll->GetOriginalFunction<_HudUpdate>(Offsets::HudUpdate);

        if (Game::Version == Game::Portal2) {
            auto fel = SAR::Find("FindElement");
            if (fel.Found) {
                using _FindElement = void*(__thiscall*)(void* thisptr, const char* pName);
                auto FindElement = reinterpret_cast<_FindElement>(fel.Address);

                auto GetHudAddr = Memory::ReadAbsoluteAddress((uintptr_t)Original::HudUpdate + Offsets::GetHud);
                using _GetHud = void*(__cdecl*)(int unk);
                auto gHUD = reinterpret_cast<_GetHud>(GetHudAddr)(-1);

                auto element = FindElement(gHUD, "CHUDChallengeStats");
                g_HUDChallengeStats = std::make_unique<VMTHook>(element);
                g_HUDChallengeStats->HookFunction((void*)Detour::GetName, Offsets::GetName);
                Original::GetName = g_HUDChallengeStats->GetOriginalFunction<_GetName>(Offsets::GetName);
            }
        } else if (Game::Version == Game::TheStanleyParable) {
            auto IN_ActivateMouse = clientdll->GetOriginalFunction<uintptr_t>(Offsets::IN_ActivateMouse);
            auto g_InputAdr = **reinterpret_cast<void***>(IN_ActivateMouse + Offsets::g_Input);
            g_Input = std::make_unique<VMTHook>(g_InputAdr);

            auto GetButtonBits = g_Input->GetOriginalFunction<uintptr_t>(Offsets::GetButtonBits);
            in_jump = *reinterpret_cast<void**>((uintptr_t)GetButtonBits + Offsets::in_jump);

            auto JoyStickApplyMovement = g_Input->GetOriginalFunction<uintptr_t>(Offsets::JoyStickApplyMovement);
            auto keyDownAddr = Memory::ReadAbsoluteAddress(JoyStickApplyMovement + Offsets::KeyDown);
            auto keyUpAddr = Memory::ReadAbsoluteAddress(JoyStickApplyMovement + Offsets::KeyUp);
            KeyDown = reinterpret_cast<_KeyDown>(keyDownAddr);
            KeyUp = reinterpret_cast<_KeyUp>(keyUpAddr);
        }

        /* // Before HudUpdate in VMT
            auto HudProcessInput = clientdll->GetOriginalFunction<uintptr_t>(Offsets::HudUpdate - 1);
            typedef void*(*_GetClientMode)();
            auto GetClientMode = reinterpret_cast<_GetClientMode>(GetAbsoluteAddress(HudProcessInput + 12));
            g_pClientMode = std::make_unique<VMTHook>(GetClientMode());

            g_pClientMode->HookFunction((void*)Detour::CreateMove, Offsets::CreateMove);
            Original::CreateMove = g_pClientMode->GetOriginalFunction<_CreateMove>(Offsets::CreateMove); */

        if (Interfaces::IClientEntityList) {
            s_EntityList = std::make_unique<VMTHook>(Interfaces::IClientEntityList);
            GetClientEntity = s_EntityList->GetOriginalFunction<_GetClientEntity>(Offsets::GetClientEntity);
        }
    }
}
}