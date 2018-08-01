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

    // engine.dll

    tickcount = 103; // CClientState::ProcessTick
    interval_per_tick = 73; // CClientState::ProcessTick
    Disconnect = 16; //  CClientState
    m_szLevelName = 36; // CEngineTool::GetCurrentMap
    AutoCompletionFunc = 37; // listdemo_CompletionFunc (TODO)
    Key_SetBinding = 60; // unbind (TODO)
    IsRunningSimulation = 12; // CEngineAPI (TODO)
    eng = 7; // CEngineAPI::IsRunningSimulation (TODO)
    HostState_OnClientConnected = 735; // CClientState::SetSignonState (TODO)
    hoststate = 9; // HostState_OnClientConnected (TODO)
    m_bLoadGame = 440; // CBaseServer::m_szLevelName (TODO)

    // client.dll

    IN_ActivateMouse = 15; // CHLClient
    g_Input = 2; // CHLClient::IN_ActivateMouse
    GetButtonBits = 2; //CInput
    JoyStickApplyMovement = 60; // CInput
    in_jump = 420; // CInput::GetButtonBits
    KeyDown = 255; // CInput::JoyStickApplyMovement
    KeyUp = 234; // CInput::JoyStickApplyMovement

    // vguimatsurface.dll

    PaintTraverseEx = 117; // CMatSystemSurface (TODO)
    StartDrawing = 193; // CMatSystemSurface::PaintTraverseEx (TODO)
    FinishDrawing = 590; // CMatSystemSurface::PaintTraverseEx (TODO)
    DrawColoredText = 159; // CMatSystemSurface
    DrawTextLen = 162; // CMatSystemSurface
}
void TheStanleyParable::LoadRules()
{
}
const char* TheStanleyParable::GetVersion()
{
    return "The Stanley Parable (TODO)";
}
