#include "INFRA.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

INFRA::INFRA()
{
    this->version = SourceGame_INFRA;
}
void INFRA::LoadOffsets()
{
    Portal2::LoadOffsets();

    using namespace Offsets;

    // engine.dll

    Dtor = 9; // ConVar
    InternalSetValue = 14; // ConVar
    InternalSetFloatValue = 15; // ConVar
    InternalSetIntValue = 16; // ConVar
    Create = 29; // ConVar
    GetLocalClient = 138; // CEngineClient::SetViewAngles
    viewangles = 35424; // CEngineClient::SetViewAngles
    GetActiveSplitScreenPlayerSlot = 129; // CEngineClient
    tickcount = 104; // CClientState::ProcessTick
    interval_per_tick = 74; // CClientState::ProcessTick
    HostState_OnClientConnected = 724; // CClientState::SetSignonState
    ConPrintEvent = 351; // CGameEventManager::FireEventIntern
    ClientCommand = 40; // CVEngineServer

    // server.dll

    FinishGravity = 33; // CPortalGameMovement
    CheckJumpButton = 35; // CPortalGameMovement
    IsRestoring = 25; // CServerGameDLL
    Think = 32; // CServerGameDLL
    NUM_ENT_ENTRIES = 16384; // CBaseEntityList::CBaseEntityList

    // client.dll

    GetAllClasses = 9; // CHLClient
    HudProcessInput = 11; // CHLClient
    HudUpdate = 12; // CHLClient
    IN_ActivateMouse = 16; // CHLClient
    PerUserInput_tSize = 388; // CInput::DecodeUserCmdFromBuffer
    m_pCommands = 380; // CInput::DecodeUserCmdFromBuffer
    CUserCmdSize = 116; // CInput::DecodeUserCmdFromBuffer
    JoyStickApplyMovement = 61; // CInput
    KeyDown = 401; // CInput::JoyStickApplyMovement
    KeyUp = 380; // CInput::JoyStickApplyMovement

    // vguimatsurface.dll

    PaintTraverseEx = 118; // CMatSystemSurface
}
const char* INFRA::Version()
{
    return "INFRA (6905)";
}
const float INFRA::Tickrate()
{
    return 120;
}
const char* INFRA::ModDir()
{
    return "infra";
}
