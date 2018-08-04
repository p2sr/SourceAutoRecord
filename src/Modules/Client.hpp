#pragma once
#include "Console.hpp"
#include "Engine.hpp"

#include "Features/Hud/InputHud.hpp"
#include "Features/Tas.hpp"

#include "Cheats.hpp"
#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

namespace Client {

Interface* g_ClientDLL;
Interface* g_pClientMode;
Interface* g_HUDChallengeStats;
Interface* s_EntityList;

using _GetClientEntity = void*(__func*)(void* thisptr, int entnum);
using _KeyDown = int(__cdecl*)(void* b, const char* c);
using _KeyUp = int(__cdecl*)(void* b, const char* c);

_GetClientEntity GetClientEntity;
_KeyDown KeyDown;
_KeyUp KeyUp;

void* in_jump;

void* GetPlayer()
{
    return GetClientEntity(s_EntityList->ThisPtr(), Engine::GetLocalPlayerIndex());
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

void Init()
{
    bool readJmp = false;
#ifdef _WIN32
    readJmp = game->version == SourceGame::TheStanleyParable
        || game->version == SourceGame::TheBeginnersGuide;
#endif

    g_ClientDLL = Interface::Create(MODULE("client"), "VClient0");
    s_EntityList = Interface::Create(MODULE("client"), "VClientEntityList0", false);

    if (g_ClientDLL) {
        g_ClientDLL->Hook(Detour::HudUpdate, Original::HudUpdate, Offsets::HudUpdate);

        if (game->version == SourceGame::Portal2) {
            auto leaderboard = Command("+leaderboard");
            if (leaderboard.GetPtr()) {
                using _GetHud = void*(__cdecl*)(int unk);
                using _FindElement = void*(__func*)(void* thisptr, const char* pName);

                auto cc_leaderboard_enable = (uintptr_t)leaderboard.GetPtr()->m_pCommandCallback;
                auto GetHud = Memory::Read<_GetHud>(cc_leaderboard_enable + Offsets::GetHud);
                auto FindElement = Memory::Read<_FindElement>(cc_leaderboard_enable + Offsets::FindElement);
                auto CHUDChallengeStats = FindElement(GetHud(-1), "CHUDChallengeStats");

                if (g_HUDChallengeStats = Interface::Create(CHUDChallengeStats)) {
                    g_HUDChallengeStats->Hook(Detour::GetName, Original::GetName, Offsets::GetName);
                }
            }
        } else if (game->version == SourceGame::TheStanleyParable) {
            auto IN_ActivateMouse = g_ClientDLL->Original(Offsets::IN_ActivateMouse, readJmp);
            auto g_InputAddr = Memory::DerefDeref<void*>(IN_ActivateMouse + Offsets::g_Input);

            if (auto g_Input = Interface::Create(g_InputAddr, false)) {
                auto GetButtonBits = g_Input->Original(Offsets::GetButtonBits, readJmp);
                Memory::Deref(GetButtonBits + Offsets::in_jump, &in_jump);

                auto JoyStickApplyMovement = g_Input->Original(Offsets::JoyStickApplyMovement, readJmp);
                Memory::Read(JoyStickApplyMovement + Offsets::KeyDown, &KeyDown);
                Memory::Read(JoyStickApplyMovement + Offsets::KeyUp, &KeyUp);
            }
        }

        auto HudProcessInput = g_ClientDLL->Original(Offsets::HudProcessInput, readJmp);
        void* clientMode = nullptr;
        if (Game::IsPortal2Engine()) {
            typedef void* (*_GetClientMode)();
            auto GetClientMode = Memory::Read<_GetClientMode>(HudProcessInput + Offsets::GetClientMode);
            clientMode = GetClientMode();
        } else {
            clientMode = Memory::DerefDeref<void*>(HudProcessInput + Offsets::GetClientMode);
        }

        if (g_pClientMode = Interface::Create(clientMode)) {
            g_pClientMode->Hook(Detour::CreateMove, Original::CreateMove, Offsets::CreateMove);
        }
    }

    if (s_EntityList) {
        GetClientEntity = s_EntityList->Original<_GetClientEntity>(Offsets::GetClientEntity, readJmp);
    }
}
void Shutdown()
{
    Interface::Delete(g_ClientDLL);
    Interface::Delete(g_pClientMode);
    Interface::Delete(g_HUDChallengeStats);
    Interface::Delete(s_EntityList);
}
}
