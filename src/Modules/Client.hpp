#pragma once
#include "Console.hpp"
#include "Engine.hpp"

#include "Features/Hud/InputHud.hpp"
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
    return GetClientEntity(s_EntityList->GetThisPtr(), Engine::GetLocalPlayerIndex());
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
        for (auto&& tas = TAS::Frames.begin(); tas != TAS::Frames.end();) {
            tas->FramesLeft--;
            if (tas->FramesLeft <= 0) {
                console->DevMsg("[%i] %s\n", Engine::currentFrame, tas->Command.c_str());
                Engine::ExecuteCommand(tas->Command.c_str());
                tas = TAS::Frames.erase(tas);
            } else {
                ++tas;
            }
        }
    }

    ++Engine::currentFrame;
    return Original::HudUpdate(thisptr, a2);
}

// ClientModeShared::CreateMove
DETOUR(CreateMove, float flInputSampleTime, CUserCmd* cmd)
{
    inputHud->SetButtonBits(cmd->buttons);

    if (cmd->command_number) {
        if (TAS2::IsPlaying) {
            TAS2::Play(cmd);
        } else if (TAS2::IsRecording) {
            TAS2::Record(cmd);
        }
    }

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
    readJmp = Game::Version == Game::TheStanleyParable
        || Game::Version == Game::TheBeginnersGuide;
#endif

    CREATE_VMT(Interfaces::IBaseClientDLL, clientdll)
    {
        HOOK(clientdll, HudUpdate);

        if (game->version == SourceGame::Portal2) {
            auto leaderboard = Command("+leaderboard");
            if (leaderboard.GetPtr()) {
                using _GetHud = void*(__cdecl*)(int unk);
                using _FindElement = void*(__func*)(void* thisptr, const char* pName);

                auto cc_leaderboard_enable = (uintptr_t)leaderboard.GetPtr()->m_pCommandCallback;
                auto GetHud = Memory::Read<_GetHud>(cc_leaderboard_enable + Offsets::GetHud);
                auto FindElement = Memory::Read<_FindElement>(cc_leaderboard_enable + Offsets::FindElement);

                auto CHUDChallengeStats = FindElement(GetHud(-1), "CHUDChallengeStats");
                CREATE_VMT(CHUDChallengeStats, g_HUDChallengeStats)
                {
                    HOOK(g_HUDChallengeStats, GetName);
                }
            }
        } else if (game->version == SourceGame::TheStanleyParable) {
            auto IN_ActivateMouse = clientdll->GetOriginal(Offsets::IN_ActivateMouse, readJmp);
            auto g_InputAddr = Memory::DerefDeref<void*>(IN_ActivateMouse + Offsets::g_Input);

            VMT g_Input;
            COPY_VMT(g_InputAddr, g_Input)
            {
                auto GetButtonBits = g_Input->GetOriginal(Offsets::GetButtonBits, readJmp);
                in_jump = Memory::Deref<void*>((uintptr_t)GetButtonBits + Offsets::in_jump);

                auto JoyStickApplyMovement = g_Input->GetOriginal(Offsets::JoyStickApplyMovement, readJmp);
                KeyDown = Memory::Read<_KeyDown>(JoyStickApplyMovement + Offsets::KeyDown);
                KeyUp = Memory::Read<_KeyUp>(JoyStickApplyMovement + Offsets::KeyUp);
            }
        }

        auto HudProcessInput = clientdll->GetOriginal(Offsets::HudProcessInput, readJmp);
        void* clientMode = nullptr;
        if (Game::IsPortal2Engine()) {
            typedef void* (*_GetClientMode)();
            auto GetClientMode = Memory::Read<_GetClientMode>(HudProcessInput + Offsets::GetClientMode);
            clientMode = GetClientMode();
        } else {
            clientMode = Memory::DerefDeref<void*>(HudProcessInput + Offsets::GetClientMode);
        }

        CREATE_VMT(clientMode, g_pClientMode)
        {
            HOOK(g_pClientMode, CreateMove);
        }
    }

    CREATE_VMT(Interfaces::IClientEntityList, s_EntityList)
    {
        GetClientEntity = s_EntityList->GetOriginal<_GetClientEntity>(Offsets::GetClientEntity, readJmp);
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
