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
    HostState_OnClientConnected = 695; // CClientState::SetSignonState

    // client.dll

    IN_ActivateMouse = 15; // CHLClient
    g_Input = 2; // CHLClient::IN_ActivateMouse
    GetButtonBits = 2; //CInput
    JoyStickApplyMovement = 60; // CInput
    in_jump = 420; // CInput::GetButtonBits
    KeyDown = 255; // CInput::JoyStickApplyMovement
    KeyUp = 234; // CInput::JoyStickApplyMovement

    // vguimatsurface.dll

    DrawColoredText = 159; // CMatSystemSurface
    DrawTextLen = 162; // CMatSystemSurface
}
void TheStanleyParable::LoadRules()
{
}
const char* TheStanleyParable::Version()
{
    return "The Stanley Parable (5454)";
}
