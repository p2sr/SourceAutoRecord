#include "TheStanleyParable.hpp"

#include "Offsets.hpp"

TheStanleyParable::TheStanleyParable()
{
    this->version = SourceGame::TheStanleyParable;
}
void TheStanleyParable::LoadOffsets()
{
    Portal2::LoadOffsets();

    using namespace Offsets;

    // engine.so

    GetActiveSplitScreenPlayerSlot = 127; // CEngineClient (TODO)
    tickcount = 74; // CClientState::ProcessTick
    interval_per_tick = 82; // CClientState::ProcessTick
    m_szLevelName = 56; // CEngineTool::GetCurrentMap
    demoplayer = 92; // CClientState::Disconnect
    demorecorder = 105; // CClientState::Disconnect
    Cbuf_AddText = 46; // CEngine::ClientCmd (TODO)
    AutoCompletionFunc = 37; // listdemo_CompletionFunc (TODO)
    Key_SetBinding = 60; // unbind (TODO)
    IsRunningSimulation = 12; // CEngineAPI (TODO)
    eng = 7; // CEngineAPI::IsRunningSimulation (TODO)
    HostState_OnClientConnected = 735; // CClientState::SetSignonState (TODO)
    hoststate = 9; // HostState_OnClientConnected (TODO)
    m_bLoadGame = 440; // CBaseServer::m_szLevelName (TODO)

    // server.so

    gpGlobals = 576; // CGameMovement::FullTossMove

    // client.so

    GetClientMode = 11; // CHLClient::HudProcessInput
    IN_ActivateMouse = 15; // CHLClient
    g_Input = 1; // CHLClient::IN_ActivateMouse
    GetButtonBits = 2; // CInput
    JoyStickApplyMovement = 64; // CInput
    in_jump = 210; // CInput::GetButtonBits
    KeyDown = 337; // CInput::JoyStickApplyMovement
    KeyUp = 384; // CInput::JoyStickApplyMovement
}
void TheStanleyParable::LoadRules()
{
}
const char* TheStanleyParable::GetVersion()
{
    return "The Stanley Parable (6130)";
}
