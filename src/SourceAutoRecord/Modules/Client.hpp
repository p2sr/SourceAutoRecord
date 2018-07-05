#pragma once
#include "vmthook/vmthook.h"

#include "Console.hpp"
#include "Engine.hpp"

#include "Features/Tas.hpp"

#include "Cheats.hpp"
#include "Game.hpp"
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

namespace Client {

VMT clientdll;
VMT g_HUDChallengeStats;
VMT s_EntityList;

using _GetClientEntity = void*(__func*)(void* thisptr, int entnum);
using _KeyDown = int(__cdecl*)(void* b, const char* c);
using _KeyUp = int(__cdecl*)(void* b, const char* c);

_GetClientEntity GetClientEntity;
_KeyDown KeyDown;
_KeyUp KeyUp;

void* in_jump;

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

// CHLClient::HudUpdate
DETOUR(HudUpdate, unsigned int a2)
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

//// CHLClient::CreateMove
//DETOUR(CreateMove, float flInputSampleTime, CUserCmd* cmd)
//{
//    return Original::CreateMove(thisptr, flInputSampleTime, cmd);
//}

// CHud::GetName
DETOUR_T(const char*, GetName)
{
    // Never allow CHud::FindElement to find this HUD
    if (Cheats::sar_disable_challenge_stats_hud.GetBool())
        return "";

    return Original::GetName(thisptr);
}

void Hook()
{
    if (SAR::NewVMT(Interfaces::IBaseClientDLL, clientdll)) {
        HOOK(clientdll, HudUpdate);

        if (Game::Version == Game::Portal2) {
            auto fel = SAR::Find("FindElement");
            if (fel.Found) {
                using _FindElement = void*(__thiscall*)(void* thisptr, const char* pName);
                auto FindElement = reinterpret_cast<_FindElement>(fel.Address);

                auto GetHudAddr = Memory::ReadAbsoluteAddress((uintptr_t)Original::HudUpdate + Offsets::GetHud);
                using _GetHud = void*(__cdecl*)(int unk);
                auto gHUD = reinterpret_cast<_GetHud>(GetHudAddr)(-1);

                auto element = FindElement(gHUD, "CHUDChallengeStats");
                if (SAR::NewVMT(element, g_HUDChallengeStats)) {
                    HOOK(g_HUDChallengeStats, GetName);
                }
            }
        } else if (Game::Version == Game::TheStanleyParable) {
            auto IN_ActivateMouse = clientdll->GetOriginalFunction<uintptr_t>(Offsets::IN_ActivateMouse);
            auto g_InputAdr = **reinterpret_cast<void***>(IN_ActivateMouse + Offsets::g_Input);
            auto g_Input = std::make_unique<VMTHook>(g_InputAdr);

            auto GetButtonBits = g_Input->GetOriginalFunction<uintptr_t>(Offsets::GetButtonBits);
            in_jump = *reinterpret_cast<void**>((uintptr_t)GetButtonBits + Offsets::in_jump);

            auto JoyStickApplyMovement = g_Input->GetOriginalFunction<uintptr_t>(Offsets::JoyStickApplyMovement);
            auto keyDownAddr = Memory::ReadAbsoluteAddress(JoyStickApplyMovement + Offsets::KeyDown);
            auto keyUpAddr = Memory::ReadAbsoluteAddress(JoyStickApplyMovement + Offsets::KeyUp);
            KeyDown = reinterpret_cast<_KeyDown>(keyDownAddr);
            KeyUp = reinterpret_cast<_KeyUp>(keyUpAddr);
        }

        if (SAR::NewVMT(Interfaces::IClientEntityList, s_EntityList)) {
            GetClientEntity = s_EntityList->GetOriginalFunction<_GetClientEntity>(Offsets::GetClientEntity);
        }
    }
}
void Unhook()
{
    UNHOOK(clientdll, HudUpdate);
    UNHOOK(g_HUDChallengeStats, GetName);

    SAR::DeleteVMT(clientdll);
    SAR::DeleteVMT(g_HUDChallengeStats);
    SAR::DeleteVMT(s_EntityList);
}
}