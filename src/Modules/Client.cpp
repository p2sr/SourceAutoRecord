#include "Client.hpp"

#include "Console.hpp"
#include "Engine.hpp"
#include "VGui.hpp"

#include "Features/Session.hpp"
#include "Features/Tas/CommandQueuer.hpp"
#include "Features/Tas/ReplaySystem.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

Variable cl_showpos;
Variable ui_loadingscreen_transition_time;
Variable hide_gun_when_holding;

REDECL(Client::HudUpdate);
REDECL(Client::CreateMove);
REDECL(Client::GetName);

void* Client::GetPlayer()
{
    return this->GetClientEntity(this->s_EntityList->ThisPtr(), engine->GetLocalPlayerIndex());
}
Vector Client::GetAbsOrigin()
{
    auto player = this->GetPlayer();
    return (player) ? *(Vector*)((uintptr_t)player + Offsets::m_vecAbsOrigin) : Vector();
}
QAngle Client::GetAbsAngles()
{
    auto player = this->GetPlayer();
    return (player) ? *(QAngle*)((uintptr_t)player + Offsets::m_angAbsRotation) : QAngle();
}
Vector Client::GetLocalVelocity()
{
    auto player = this->GetPlayer();
    return (player) ? *(Vector*)((uintptr_t)player + Offsets::m_vecVelocity) : Vector();
}
int Client::GetFlags()
{
    auto player = this->GetPlayer();
    return (player) ? *(int*)((uintptr_t)player + Offsets::GetFlags) : 0;
}

// CHLClient::HudUpdate
DETOUR(Client::HudUpdate, unsigned int a2)
{
    if (tasQueuer->isRunning) {
        for (auto&& tas = tasQueuer->frames.begin(); tas != tasQueuer->frames.end();) {
            --tas->framesLeft;

            if (tas->framesLeft <= 0) {
                console->DevMsg("[%i] %s\n", session->currentFrame, tas->command.c_str());

                if (sar.game->version & SourceGame_Portal2Engine) {
                    engine->Cbuf_AddText(tas->splitScreen, tas->command.c_str(), 0);
                } else if (sar.game->version & SourceGame_HalfLife2Engine) {
                    engine->AddText(engine->s_CommandBuffer, tas->command.c_str(), 0);
                }

                tas = tasQueuer->frames.erase(tas);
            } else {
                ++tas;
            }
        }
    }

    ++session->currentFrame;
    return Client::HudUpdate(thisptr, a2);
}

// ClientModeShared::CreateMove
DETOUR(Client::CreateMove, float flInputSampleTime, CUserCmd* cmd)
{
    vgui->inputHud->SetButtonBits(cmd->buttons);

    if (cmd->command_number) {
        if (tasReplaySystem->IsPlaying()) {
            auto replay = tasReplaySystem->GetCurrentReplay();
            if (replay->Ended()) {
                tasReplaySystem->Stop();
            } else {
                replay->Play(cmd, 0);
            }
        } else if (tasReplaySystem->IsRecording()) {
            tasReplaySystem->GetCurrentReplay()->Record(cmd, 0);
        }
    }

    return Client::CreateMove(thisptr, flInputSampleTime, cmd);
}

// CHud::GetName
DETOUR_T(const char*, Client::GetName)
{
    // Never allow CHud::FindElement to find this HUD
    if (sar_disable_challenge_stats_hud.GetBool())
        return "";

    return Client::GetName(thisptr);
}

bool Client::Init()
{
    bool readJmp = false;
#ifdef _WIN32
    readJmp = sar.game->version == SourceGame_TheStanleyParable
        || sar.game->version == SourceGame_TheBeginnersGuide;
#endif

    this->g_ClientDLL = Interface::Create(this->Name(), "VClient0");
    this->s_EntityList = Interface::Create(this->Name(), "VClientEntityList0", false);

    if (this->g_ClientDLL) {
        this->g_ClientDLL->Hook(Client::HudUpdate_Hook, Client::HudUpdate, Offsets::HudUpdate);

        if (sar.game->version == SourceGame_Portal2) {
            auto leaderboard = Command("+leaderboard");
            if (!!leaderboard) {
                using _GetHud = void*(__cdecl*)(int unk);
                using _FindElement = void*(__func*)(void* thisptr, const char* pName);

                auto cc_leaderboard_enable = (uintptr_t)leaderboard.ThisPtr()->m_pCommandCallback;
                auto GetHud = Memory::Read<_GetHud>(cc_leaderboard_enable + Offsets::GetHud);
                auto FindElement = Memory::Read<_FindElement>(cc_leaderboard_enable + Offsets::FindElement);
                auto CHUDChallengeStats = FindElement(GetHud(-1), "CHUDChallengeStats");

                if (this->g_HUDChallengeStats = Interface::Create(CHUDChallengeStats)) {
                    this->g_HUDChallengeStats->Hook(Client::GetName_Hook, Client::GetName, Offsets::GetName);
                }
            }
        } else if (sar.game->version == SourceGame_TheStanleyParable) {
            auto IN_ActivateMouse = this->g_ClientDLL->Original(Offsets::IN_ActivateMouse, readJmp);
            auto g_InputAddr = Memory::DerefDeref<void*>(IN_ActivateMouse + Offsets::g_Input);

            if (auto g_Input = Interface::Create(g_InputAddr, false)) {
                auto GetButtonBits = g_Input->Original(Offsets::GetButtonBits, readJmp);
                Memory::Deref(GetButtonBits + Offsets::in_jump, &this->in_jump);

                auto JoyStickApplyMovement = g_Input->Original(Offsets::JoyStickApplyMovement, readJmp);
                Memory::Read(JoyStickApplyMovement + Offsets::KeyDown, &this->KeyDown);
                Memory::Read(JoyStickApplyMovement + Offsets::KeyUp, &this->KeyUp);
            }
        }

        auto HudProcessInput = this->g_ClientDLL->Original(Offsets::HudProcessInput, readJmp);
        void* clientMode = nullptr;
        if (sar.game->version & SourceGame_Portal2Engine) {
            typedef void* (*_GetClientMode)();
            auto GetClientMode = Memory::Read<_GetClientMode>(HudProcessInput + Offsets::GetClientMode);
            clientMode = GetClientMode();
        } else if (sar.game->version & SourceGame_HalfLife2Engine) {
            clientMode = Memory::DerefDeref<void*>(HudProcessInput + Offsets::GetClientMode);
        }

        if (this->g_pClientMode = Interface::Create(clientMode)) {
            this->g_pClientMode->Hook(Client::CreateMove_Hook, Client::CreateMove, Offsets::CreateMove);
        }
    }

    if (this->s_EntityList) {
        this->GetClientEntity = this->s_EntityList->Original<_GetClientEntity>(Offsets::GetClientEntity, readJmp);
    }

    return this->hasLoaded = this->g_ClientDLL && this->s_EntityList;
}
void Client::Shutdown()
{
    Interface::Delete(this->g_ClientDLL);
    Interface::Delete(this->g_pClientMode);
    Interface::Delete(this->g_HUDChallengeStats);
    Interface::Delete(this->s_EntityList);
}

Client* client;
