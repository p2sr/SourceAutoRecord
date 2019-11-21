#include "TheStanleyParable.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

TheStanleyParable::TheStanleyParable()
{
    this->version = SourceGame_TheStanleyParable;
}
void TheStanleyParable::LoadOffsets()
{
    Portal2::LoadOffsets();

    using namespace Offsets;

    // engine.so

    GetLocalClient = 92; // CEngineClient::SetViewAngles
    viewangles = 19084; // CEngineClient::SetViewAngles
    s_CommandBuffer = 71; // Cbuf_AddText
    //m_bWaitEnabled = 8264; // CCommandBuffer::AddText
    tickcount = 74; // CClientState::ProcessTick
    interval_per_tick = 82; // CClientState::ProcessTick
    HostState_OnClientConnected = 1523; // CClientState::SetSignonState
    demoplayer = 92; // CClientState::Disconnect
    demorecorder = 105; // CClientState::Disconnect
    m_szLevelName = 56; // CEngineTool::GetCurrentMap
    Key_SetBinding = 59; // unbind
    net_time = 21; // CDemoRecorder::GetRecordingTick

    // server.so

    NUM_ENT_ENTRIES = 8192; // CBaseEntityList::CBaseEntityList
    m_pSurfaceData = 4120; // CGameMovement::CheckJumpButton

    // client.so

    GetClientMode = 11; // CHLClient::HudProcessInput
    m_pCommands = 228; // CInput::DecodeUserCmdFromBuffer
    IN_ActivateMouse = 15; // CHLClient
    g_Input = 1; // CHLClient::IN_ActivateMouse
    GetButtonBits = 2; // CInput
    JoyStickApplyMovement = 64; // CInput
    in_jump = 210; // CInput::GetButtonBits
    KeyDown = 337; // CInput::JoyStickApplyMovement
    KeyUp = 384; // CInput::JoyStickApplyMovement

    // vguimatsurface.so

    StartDrawing = 692; // CMatSystemSurface::PaintTraverseEx
    FinishDrawing = 627; // CMatSystemSurface::PaintTraverseEx
}
const char* TheStanleyParable::Version()
{
    return "The Stanley Parable (6130)";
}
const char* TheStanleyParable::ModDir()
{
    return "thestanleyparable";
}
