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

    // engine.dll

    GetLocalClient = 138; // CEngineClient::SetViewAngles
    viewangles = 19112; // CEngineClient::SetViewAngles
    //m_bWaitEnabled = 8264; // CCommandBuffer::AddText
    tickcount = 103; // CClientState::ProcessTick
    interval_per_tick = 73; // CClientState::ProcessTick
    HostState_OnClientConnected = 695; // CClientState::SetSignonStatey

    // client.dll

    JoyStickApplyMovement = 60; // CInput
    in_jump = 420; // CInput::GetButtonBits
    KeyDown = 255; // CInput::JoyStickApplyMovement
    KeyUp = 234; // CInput::JoyStickApplyMovement

    // vguimatsurface.dll

    DrawColoredText = 159; // CMatSystemSurface
    DrawTextLen = 162; // CMatSystemSurface
}
const char* TheStanleyParable::Version()
{
    return "The Stanley Parable (5454)";
}
const char* TheStanleyParable::Process()
{
    return "stanley.exe";
}
