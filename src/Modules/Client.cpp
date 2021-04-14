#include "Client.hpp"

#include <cstdarg>
#include <cstdint>
#include <cstring>

#include "Features/Camera.hpp"
#include "Features/FovChanger.hpp"
#include "Features/Hud/InputHud.hpp"
#include "Features/Imitator.hpp"
#include "Features/OffsetFinder.hpp"
#include "Features/ReplaySystem/ReplayPlayer.hpp"
#include "Features/ReplaySystem/ReplayProvider.hpp"
#include "Features/ReplaySystem/ReplayRecorder.hpp"
#include "Features/Session.hpp"
#include "Features/Stats/Sync.hpp"
#include "Features/Tas/AutoStrafer.hpp"
#include "Features/Tas/CommandQueuer.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Features/Stats/ZachStats.hpp"
#include "Features/NetMessage.hpp"

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
Variable crosshairVariable;
Variable cl_fov;

Variable sar_disable_coop_score_hud("sar_disable_coop_score_hud", "0", "Disables the coop score HUD which appears in demo playback.\n");

REDECL(Client::HudUpdate);
REDECL(Client::LevelInitPreEntity);
REDECL(Client::CreateMove);
REDECL(Client::CreateMove2);
REDECL(Client::GetName);
REDECL(Client::ShouldDraw_BasicInfo);
REDECL(Client::MsgFunc_SayText2);
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

DECL_CVAR_CALLBACK(cl_fov)
{
    if (engine->demoplayer->IsPlaying())
        fovChanger->Force();
}

void* Client::GetPlayer(int index)
{
    return this->GetClientEntity(this->s_EntityList->ThisPtr(), index);
}
void Client::CalcButtonBits(int nSlot, int& bits, int in_button, int in_ignore, kbutton_t* button, bool reset)
{
    auto pButtonState = &button->m_PerUser[nSlot];
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

bool Client::ShouldDrawCrosshair()
{
    if (!crosshairVariable.GetBool()) {
        crosshairVariable.SetValue(1);
        auto value = this->ShouldDraw(this->g_HUDQuickInfo->ThisPtr());
        crosshairVariable.SetValue(0);
        return value;
    }

    return this->ShouldDraw(this->g_HUDQuickInfo->ThisPtr());
}

void Client::Chat(TextColor color, const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    char data[1024];
    vsnprintf(data, sizeof(data), fmt, argptr);
    va_end(argptr);
    client->ChatPrintf(client->g_HudChat->ThisPtr(), 0, 0, "%c%s", color, data);
}

void Client::QueueChat(TextColor color, const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    char data[1024];
    vsnprintf(data, sizeof data, fmt, argptr);
    va_end(argptr);
    this->chatQueue.push_back(std::pair(color, std::string(data)));
}

void Client::FlushChatQueue()
{
    for (auto& s : this->chatQueue) {
        this->Chat(s.first, "%s", s.second.c_str());
    }
    this->chatQueue.clear();
}

float Client::GetCMTimer()
{
    if (sv_bonus_challenge.GetBool()) {
        uintptr_t player = (uintptr_t)client->GetPlayer(1);
        if (player) {
            return *(float*)(player + Offsets::m_StatsThisLevel + 12) - speedrun->GetIntervalPerTick();
        }
    }
    return 0.0f;
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

// CHLClient::LevelInitPreEntity
DETOUR(Client::LevelInitPreEntity, const char *levelName)
{
    client->lastLevelName = std::string(levelName);
    return Client::LevelInitPreEntity(thisptr, levelName);
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
        if (engine->IsCoop() && engine->IsOrange())
            inputHud.SetButtonBits(1, cmd->buttons);
        else
            inputHud.SetButtonBits(0, cmd->buttons);
    }

    if (sv_cheats.GetBool() && engine->hoststate->m_activeGame) {
        camera->OverrideMovement(cmd);
    }

    if (sar_strafesync.GetBool()) {
        synchro->UpdateSync(cmd);
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

// CHudMultiplayerBasicInfo::ShouldDraw
DETOUR_T(bool, Client::ShouldDraw_BasicInfo)
{
    if (sar_disable_coop_score_hud.GetBool()) {
        return false;
    }

    return Client::ShouldDraw_BasicInfo(thisptr);
}

// CHudChat::MsgFunc_SayText2
DETOUR(Client::MsgFunc_SayText2, bf_read &msg)
{
    // copy old state in case we need to recover it
    bf_read pre = msg;

    // skip client id
    msg.ReadUnsigned(8);

    std::string str = "";
    while (true) {
        char c = (char)(uint8_t)msg.ReadUnsigned(8);
        if (!c) break;
        str += c;
    }

    if (NetMessage::ChatData(str)) {
        // skip the other crap, just in case it matters
        msg.ReadUnsigned(8);
        return 0;
    }

    msg = pre;

    return Client::MsgFunc_SayText2(thisptr, msg);
}

// CInput::DecodeUserCmdFromBuffer
DETOUR(Client::DecodeUserCmdFromBuffer, int nSlot, int buf, signed int sequence_number)
{
    auto result = Client::DecodeUserCmdFromBuffer(thisptr, nSlot, buf, sequence_number);

    auto m_pCommands = *reinterpret_cast<uintptr_t*>((uintptr_t)thisptr + nSlot * Offsets::PerUserInput_tSize + Offsets::m_pCommands);
    auto cmd = reinterpret_cast<CUserCmd*>(m_pCommands + Offsets::CUserCmdSize * (sequence_number % Offsets::MULTIPLAYER_BACKUP));

    if (nSlot == 0) {
        // A bit weird - for some reason, when playing back Orange
        // demos, nSlot is 0 even though the player's actual slot
        // (including in HUD stuff) is 1. This works as a workaround
        inputHud.SetButtonBits(0, cmd->buttons);
        inputHud.SetButtonBits(1, cmd->buttons);
    } else if (nSlot == 1) {
        inputHud.SetButtonBits(1, cmd->buttons);
    }

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

static _CommandCallback originalLeaderboardCallback;

static void LeaderboardCallback(const CCommand& args)
{
    // There's not really much rhyme or reason behind this check, it's
    // just that this is the specific command the game runs at the end
    if (sar_challenge_autostop.GetBool() && sv_bonus_challenge.GetBool() && args.ArgC() == 2 && !strcmp(args[1], "1")) {
        engine->ExecuteCommand("stop");
    }

    if (networkManager.isConnected && sv_bonus_challenge.GetBool()) {
        networkManager.NotifySpeedrunFinished(true);
    }

    if (!zachStats->GetTriggers().empty()) {
        ZachStats::Output(zachStats->GetStream(), client->GetCMTimer());
    }

    originalLeaderboardCallback(args);
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
        this->g_ClientDLL->Hook(Client::LevelInitPreEntity_Hook, Client::LevelInitPreEntity, Offsets::LevelInitPreEntity);

        if (sar.game->Is(SourceGame_Portal2Game)) {
            auto leaderboard = Command("+leaderboard");
            if (!!leaderboard) {
                using _GetHud = void*(__cdecl*)(int unk);
                using _FindElement = void*(__rescall*)(void* thisptr, const char* pName);

                auto cc_leaderboard_enable = (uintptr_t)leaderboard.ThisPtr()->m_pCommandCallback;
                auto GetHud = Memory::Read<_GetHud>(cc_leaderboard_enable + Offsets::GetHud);
                auto FindElement = Memory::Read<_FindElement>(cc_leaderboard_enable + Offsets::FindElement);
                auto CHUDChallengeStats = FindElement(GetHud(-1), "CHUDChallengeStats");

                Command::Hook("leaderboard_open", &LeaderboardCallback, originalLeaderboardCallback);

                if (this->g_HUDChallengeStats = Interface::Create(CHUDChallengeStats)) {
                    this->g_HUDChallengeStats->Hook(Client::GetName_Hook, Client::GetName, Offsets::GetName);
                }

                auto CHUDQuickInfo = FindElement(GetHud(-1), "CHUDQuickInfo");

                if (this->g_HUDQuickInfo = Interface::Create(CHUDQuickInfo)) {
                    this->ShouldDraw = this->g_HUDQuickInfo->Original<_ShouldDraw>(Offsets::ShouldDraw, readJmp);
                }

                auto CHudChat = FindElement(GetHud(-1), "CHudChat");
                if (this->g_HudChat = Interface::Create(CHudChat)) {
                    this->ChatPrintf = g_HudChat->Original<_ChatPrintf>(Offsets::ChatPrintf);
                    this->g_HudChat->Hook(Client::MsgFunc_SayText2_Hook, Client::MsgFunc_SayText2, Offsets::MsgFunc_SayText2);
                }

                auto CHudMultiplayerBasicInfo = FindElement(GetHud(-1), "CHudMultiplayerBasicInfo");
                if (this->g_HudMultiplayerBasicInfo = Interface::Create(CHudMultiplayerBasicInfo)) {
                    this->g_HudMultiplayerBasicInfo->Hook(Client::ShouldDraw_BasicInfo_Hook, Client::ShouldDraw_BasicInfo, Offsets::ShouldDraw);
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
                } else if (sar.game->Is(SourceGame_Portal2Game)) {
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
            if (sar.game->Is(SourceGame_Portal2Game)) {
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
    offsetFinder->ClientSide("CPortal_Player", "m_StatsThisLevel", &Offsets::m_StatsThisLevel);

    cl_showpos = Variable("cl_showpos");
    cl_sidespeed = Variable("cl_sidespeed");
    cl_forwardspeed = Variable("cl_forwardspeed");
    crosshairVariable = Variable("crosshair");

    CVAR_HOOK_AND_CALLBACK(cl_fov);

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
    Interface::Delete(this->g_HUDQuickInfo);
    Interface::Delete(this->g_HudChat);
    Interface::Delete(this->g_HudMultiplayerBasicInfo);
    Command::Unhook("playvideo_end_level_transition", Client::playvideo_end_level_transition_callback);
}

Client* client;
