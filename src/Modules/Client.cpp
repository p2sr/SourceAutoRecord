#include "Client.hpp"

#include <cstdint>
#include <cstring>

#include "Features/Hud/InputHud.hpp"
#include "Features/Imitator.hpp"
#include "Features/OffsetFinder.hpp"
#include "Features/ReplaySystem/ReplayPlayer.hpp"
#include "Features/ReplaySystem/ReplayProvider.hpp"
#include "Features/ReplaySystem/ReplayRecorder.hpp"
#include "Features/Session.hpp"
#include "Features/Tas/AutoStrafer.hpp"
#include "Features/Tas/CommandQueuer.hpp"
#include "Features/Camera.hpp"

#include "Console.hpp"
#include "Engine.hpp"
#include "Server.hpp"

#include "Command.hpp"
#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

Variable cl_showpos;
Variable cl_sidespeed;
Variable cl_forwardspeed;
Variable in_forceuser;
Variable cl_fov;

REDECL(Client::HudUpdate);
REDECL(Client::CreateMove);
REDECL(Client::CreateMove2);
REDECL(Client::GetName);
REDECL(Client::DecodeUserCmdFromBuffer);
REDECL(Client::DecodeUserCmdFromBuffer2);
REDECL(Client::CInput_CreateMove);
REDECL(Client::GetButtonBits);
REDECL(Client::playvideo_end_level_transition_callback);
REDECL(Client::OverrideView);

MDECL(Client::GetAbsOrigin, Vector, C_m_vecAbsOrigin);
MDECL(Client::GetAbsAngles, QAngle, C_m_angAbsRotation);
MDECL(Client::GetLocalVelocity, Vector, C_m_vecVelocity);
MDECL(Client::GetViewOffset, Vector, C_m_vecViewOffset);

void* Client::GetPlayer(int index)
{
    return this->GetClientEntity(this->s_EntityList->ThisPtr(), index);
}
void Client::CalcButtonBits(int nSlot, int& bits, int in_button, int in_ignore, kbutton_t* button, bool reset)
{
    auto pButtonState = &button->GetPerUser(nSlot);
    if (pButtonState->state & 3) {
        bits |= in_button;
    }

    int clearmask = ~2;
    if (in_ignore & in_button) {
        clearmask = ~3;
    }

    if (reset) {
        pButtonState->state &= clearmask;
    }
}

// CHLClient::HudUpdate
DETOUR(Client::HudUpdate, unsigned int a2)
{
    if (cmdQueuer->isRunning) {
        for (auto&& tas = cmdQueuer->frames.begin(); tas != cmdQueuer->frames.end();) {
            --tas->framesLeft;

            if (tas->framesLeft <= 0) {
                console->DevMsg("[%i] %s\n", session->currentFrame, tas->command.c_str());

                if (sar.game->Is(SourceGame_Portal2Engine)) {
                    if (engine->GetMaxClients() <= 1) {
                        engine->Cbuf_AddText(tas->splitScreen, tas->command.c_str(), 0);
                    } else {
                        auto entity = engine->PEntityOfEntIndex(tas->splitScreen + 1);
                        if (entity && !entity->IsFree() && server->IsPlayer(entity->m_pUnk)) {
                            engine->ClientCommand(nullptr, entity, tas->command.c_str());
                        }
                    }
                } else if (sar.game->Is(SourceGame_HalfLife2Engine)) {
                    engine->AddText(engine->s_CommandBuffer, tas->command.c_str(), 0);
                }

                tas = cmdQueuer->frames.erase(tas);
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
    if (cmd->command_number) {
        if (replayPlayer1->IsPlaying()) {
            replayPlayer1->Play(replayProvider->GetCurrentReplay(), cmd);
        } else if (replayRecorder1->IsRecording()) {
            replayRecorder1->Record(replayProvider->GetCurrentReplay(), cmd);
        }
    }

    if (sar_mimic.isRegistered && sar_mimic.GetBool()) {
        imitator->Save(cmd);
    }

    if (!in_forceuser.isReference || (in_forceuser.isReference && !in_forceuser.GetBool())) {
        inputHud.SetButtonBits(0, cmd->buttons);
    }

    if (sv_cheats.GetBool() && engine->hoststate->m_activeGame) {
        camera->OverrideMovement(cmd);
    }
    

    return Client::CreateMove(thisptr, flInputSampleTime, cmd);
}
DETOUR(Client::CreateMove2, float flInputSampleTime, CUserCmd* cmd)
{
    if (cmd->command_number) {
        if (replayPlayer2->IsPlaying()) {
            replayPlayer2->Play(replayProvider->GetCurrentReplay(), cmd);
        } else if (replayRecorder2->IsRecording()) {
            replayRecorder2->Record(replayProvider->GetCurrentReplay(), cmd);
        }
    }

    if (sar_mimic.GetBool() && (!sv_bonus_challenge.GetBool() || sv_cheats.GetBool())) {
        imitator->Modify(cmd);
    }

    if (in_forceuser.GetBool()) {
        inputHud.SetButtonBits(1, cmd->buttons);
    }

    return Client::CreateMove2(thisptr, flInputSampleTime, cmd);
}

// CHud::GetName
DETOUR_T(const char*, Client::GetName)
{
    // Never allow CHud::FindElement to find this HUD
    if (sar_disable_challenge_stats_hud.GetBool())
        return "";

    return Client::GetName(thisptr);
}

// CInput::DecodeUserCmdFromBuffer
DETOUR(Client::DecodeUserCmdFromBuffer, int nSlot, int buf, signed int sequence_number)
{
    auto result = Client::DecodeUserCmdFromBuffer(thisptr, nSlot, buf, sequence_number);

    auto m_pCommands = *reinterpret_cast<uintptr_t*>((uintptr_t)thisptr + nSlot * Offsets::PerUserInput_tSize + Offsets::m_pCommands);
    auto cmd = reinterpret_cast<CUserCmd*>(m_pCommands + Offsets::CUserCmdSize * (sequence_number % Offsets::MULTIPLAYER_BACKUP));

    inputHud.SetButtonBits(0, cmd->buttons);

    return result;
}
DETOUR(Client::DecodeUserCmdFromBuffer2, int buf, signed int sequence_number)
{
    auto result = Client::DecodeUserCmdFromBuffer2(thisptr, buf, sequence_number);

    auto m_pCommands = *reinterpret_cast<uintptr_t*>((uintptr_t)thisptr + Offsets::m_pCommands);
    auto cmd = reinterpret_cast<CUserCmd*>(m_pCommands + Offsets::CUserCmdSize * (sequence_number % Offsets::MULTIPLAYER_BACKUP));

    inputHud.SetButtonBits(1, cmd->buttons);

    return result;
}

// CInput::CreateMove
DETOUR(Client::CInput_CreateMove, int sequence_number, float input_sample_frametime, bool active)
{
    auto originalValue = 0;
    if (sar_tas_ss_forceuser.GetBool()) {
        originalValue = in_forceuser.GetInt();
        in_forceuser.SetValue(GET_SLOT());
    }

    auto result = Client::CInput_CreateMove(thisptr, sequence_number, input_sample_frametime, active);

    if (sar_tas_ss_forceuser.GetBool()) {
        in_forceuser.SetValue(originalValue);
    }

    return result;
}

// CInput::GetButtonBits
DETOUR(Client::GetButtonBits, bool bResetState)
{
    auto bits = Client::GetButtonBits(thisptr, bResetState);

    client->CalcButtonBits(GET_SLOT(), bits, IN_AUTOSTRAFE, 0, &autoStrafer->in_autostrafe, bResetState);

    return bits;
}

DETOUR_COMMAND(Client::playvideo_end_level_transition)
{
    console->DevMsg("%s\n", args.m_pArgSBuffer);
    session->Ended();

    return Client::playvideo_end_level_transition_callback(args);
}

DETOUR(Client::OverrideView, CPortalViewSetup1* m_View)
{
    camera->OverrideView(m_View);
    return Client::OverrideView(thisptr, m_View);
}

bool Client::Init()
{
    bool readJmp = false;
#ifdef _WIN32
    readJmp = sar.game->Is(SourceGame_TheStanleyParable | SourceGame_TheBeginnersGuide);
#endif

    this->g_ClientDLL = Interface::Create(this->Name(), "VClient0");
    this->s_EntityList = Interface::Create(this->Name(), "VClientEntityList0", false);

    if (this->g_ClientDLL) {
        this->GetAllClasses = this->g_ClientDLL->Original<_GetAllClasses>(Offsets::GetAllClasses, readJmp);

        this->g_ClientDLL->Hook(Client::HudUpdate_Hook, Client::HudUpdate, Offsets::HudUpdate);

        if (sar.game->Is(SourceGame_Portal2)) {
            auto leaderboard = Command("+leaderboard");
            if (!!leaderboard) {
                using _GetHud = void*(__cdecl*)(int unk);
                using _FindElement = void*(__rescall*)(void* thisptr, const char* pName);

                auto cc_leaderboard_enable = (uintptr_t)leaderboard.ThisPtr()->m_pCommandCallback;
                auto GetHud = Memory::Read<_GetHud>(cc_leaderboard_enable + Offsets::GetHud);
                auto FindElement = Memory::Read<_FindElement>(cc_leaderboard_enable + Offsets::FindElement);
                auto CHUDChallengeStats = FindElement(GetHud(-1), "CHUDChallengeStats");

                if (this->g_HUDChallengeStats = Interface::Create(CHUDChallengeStats)) {
                    this->g_HUDChallengeStats->Hook(Client::GetName_Hook, Client::GetName, Offsets::GetName);
                }
            }
        }

        auto IN_ActivateMouse = this->g_ClientDLL->Original(Offsets::IN_ActivateMouse, readJmp);
        auto g_InputAddr = Memory::DerefDeref<void*>(IN_ActivateMouse + Offsets::g_Input);

        if (g_Input = Interface::Create(g_InputAddr)) {
            if (sar.game->Is(SourceGame_Portal2Engine)) {
                g_Input->Hook(Client::DecodeUserCmdFromBuffer_Hook, Client::DecodeUserCmdFromBuffer, Offsets::DecodeUserCmdFromBuffer);
                g_Input->Hook(Client::GetButtonBits_Hook, Client::GetButtonBits, Offsets::GetButtonBits);

                auto JoyStickApplyMovement = g_Input->Original(Offsets::JoyStickApplyMovement, readJmp);
                Memory::Read(JoyStickApplyMovement + Offsets::KeyDown, &this->KeyDown);
                Memory::Read(JoyStickApplyMovement + Offsets::KeyUp, &this->KeyUp);

                if (sar.game->Is(SourceGame_TheStanleyParable)) {
                    auto GetButtonBits = g_Input->Original(Offsets::GetButtonBits, readJmp);
                    Memory::Deref(GetButtonBits + Offsets::in_jump, &this->in_jump);
                } else if (sar.game->Is(SourceGame_Portal2 | SourceGame_ApertureTag)) {
                    in_forceuser = Variable("in_forceuser");
                    if (!!in_forceuser && this->g_Input) {
                        this->g_Input->Hook(CInput_CreateMove_Hook, CInput_CreateMove, Offsets::GetButtonBits + 1);
                    }

                    Command::Hook("playvideo_end_level_transition", Client::playvideo_end_level_transition_callback_hook, Client::playvideo_end_level_transition_callback);
                }
            } else {
                g_Input->Hook(Client::DecodeUserCmdFromBuffer2_Hook, Client::DecodeUserCmdFromBuffer2, Offsets::DecodeUserCmdFromBuffer);
            }
        }

        auto HudProcessInput = this->g_ClientDLL->Original(Offsets::HudProcessInput, readJmp);
        void* clientMode = nullptr;
        void* clientMode2 = nullptr;
        if (sar.game->Is(SourceGame_Portal2Engine)) {
            if (sar.game->Is(SourceGame_Portal2 | SourceGame_ApertureTag)) {
                auto GetClientMode = Memory::Read<uintptr_t>(HudProcessInput + Offsets::GetClientMode);
                auto g_pClientMode = Memory::Deref<uintptr_t>(GetClientMode + Offsets::g_pClientMode);
                clientMode = Memory::Deref<void*>(g_pClientMode);
                clientMode2 = Memory::Deref<void*>(g_pClientMode + sizeof(void*));
            } else {
                typedef void* (*_GetClientMode)();
                auto GetClientMode = Memory::Read<_GetClientMode>(HudProcessInput + Offsets::GetClientMode);
                clientMode = GetClientMode();
            }
        } else if (sar.game->Is(SourceGame_HalfLife2Engine)) {
            clientMode = Memory::DerefDeref<void*>(HudProcessInput + Offsets::GetClientMode);
        }

        if (this->g_pClientMode = Interface::Create(clientMode)) {
            this->g_pClientMode->Hook(Client::CreateMove_Hook, Client::CreateMove, Offsets::CreateMove);
            if (sar.game->Is(SourceGame_Portal2Engine)) {
                this->g_pClientMode->Hook(Client::OverrideView_Hook, Client::OverrideView, Offsets::OverrideView);
            }
        }

        if (this->g_pClientMode2 = Interface::Create(clientMode2)) {
            this->g_pClientMode2->Hook(Client::CreateMove2_Hook, Client::CreateMove2, Offsets::CreateMove);
        }
    }

    if (this->s_EntityList) {
        this->GetClientEntity = this->s_EntityList->Original<_GetClientEntity>(Offsets::GetClientEntity, readJmp);
    }

    offsetFinder->ClientSide("CBasePlayer", "m_vecVelocity[0]", &Offsets::C_m_vecVelocity);
    offsetFinder->ClientSide("CBasePlayer", "m_vecViewOffset[0]", &Offsets::C_m_vecViewOffset);

    cl_showpos = Variable("cl_showpos");
    cl_sidespeed = Variable("cl_sidespeed");
    cl_forwardspeed = Variable("cl_forwardspeed");
    cl_fov = Variable("cl_fov");

    return this->hasLoaded = this->g_ClientDLL && this->s_EntityList;
}
void Client::Shutdown()
{
    Interface::Delete(this->g_ClientDLL);
    Interface::Delete(this->g_pClientMode);
    Interface::Delete(this->g_pClientMode2);
    Interface::Delete(this->g_HUDChallengeStats);
    Interface::Delete(this->s_EntityList);
    Interface::Delete(this->g_Input);
    Command::Unhook("playvideo_end_level_transition", Client::playvideo_end_level_transition_callback);
}

Client* client;
