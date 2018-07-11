#pragma once
#include "vmthook/vmthook.h"

#include "Console.hpp"
#include "Engine.hpp"

#include "Features/InputHud.hpp"
#include "Features/Tas.hpp"

#include "Cheats.hpp"
#include "Game.hpp"
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

namespace Client {

VMT clientdll;
VMT g_pClientMode;
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
                Console::DevMsg("[%i] %s\n", Engine::CurrentFrame, tas->Command.c_str());
                Engine::ExecuteCommand(tas->Command.c_str());
                tas = TAS::Frames.erase(tas);
            } else {
                tas++;
            }
        }
    }

    Engine::CurrentFrame++;
    return Original::HudUpdate(thisptr, a2);
}

// ClientModeShared::CreateMove
DETOUR(CreateMove, float flInputSampleTime, CUserCmd* cmd)
{
    InputHud::SetButtonBits(cmd->buttons);
    return Original::CreateMove(thisptr, flInputSampleTime, cmd);
}

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
    bool readJmp = false;
#ifdef _WIN32
    readJmp = Game::Version == Game::TheStanleyParable;
#endif

    CREATE_VMT(Interfaces::IBaseClientDLL, clientdll) {
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
                CREATE_VMT(element, g_HUDChallengeStats) {
                    HOOK(g_HUDChallengeStats, GetName);
                }
            }
        } else if (Game::Version == Game::TheStanleyParable) {
            VMT g_Input;
            auto IN_ActivateMouse = clientdll->GetOriginalFunction<uintptr_t>(Offsets::IN_ActivateMouse, readJmp);
            auto g_InputAddr = **reinterpret_cast<void***>(IN_ActivateMouse + Offsets::g_Input);

            CREATE_VMT(g_InputAddr, g_Input) {
                auto GetButtonBits = g_Input->GetOriginalFunction<uintptr_t>(Offsets::GetButtonBits, readJmp);
                in_jump = *reinterpret_cast<void**>((uintptr_t)GetButtonBits + Offsets::in_jump);

                auto JoyStickApplyMovement = g_Input->GetOriginalFunction<uintptr_t>(Offsets::JoyStickApplyMovement, readJmp);
                auto keyDownAddr = Memory::ReadAbsoluteAddress(JoyStickApplyMovement + Offsets::KeyDown);
                auto keyUpAddr = Memory::ReadAbsoluteAddress(JoyStickApplyMovement + Offsets::KeyUp);
                KeyDown = reinterpret_cast<_KeyDown>(keyDownAddr);
                KeyUp = reinterpret_cast<_KeyUp>(keyUpAddr);
            }
        }

        auto HudProcessInput = clientdll->GetOriginalFunction<uintptr_t>(Offsets::HudProcessInput);
        void* clientMode = nullptr;
        if (Game::IsPortal2Engine()) {
            typedef void*(*_GetClientMode)();
            auto GetClientMode = reinterpret_cast<_GetClientMode>(Memory::ReadAbsoluteAddress(HudProcessInput + Offsets::GetClientMode));
            clientMode = GetClientMode();
        } else {
            clientMode = **reinterpret_cast<void***>(HudProcessInput + Offsets::GetClientMode);
        }

        CREATE_VMT(clientMode, g_pClientMode) {
            HOOK(g_pClientMode, CreateMove);
        }
    }

    CREATE_VMT(Interfaces::IClientEntityList, s_EntityList) {
        GetClientEntity = s_EntityList->GetOriginalFunction<_GetClientEntity>(Offsets::GetClientEntity, readJmp);
    }
}
void Unhook()
{
    UNHOOK(clientdll, HudUpdate);
    UNHOOK(g_pClientMode, CreateMove);
    UNHOOK(g_HUDChallengeStats, GetName);

    DELETE_VMT(clientdll);
    DELETE_VMT(g_pClientMode);
    DELETE_VMT(g_HUDChallengeStats);
    DELETE_VMT(s_EntityList);
}
}