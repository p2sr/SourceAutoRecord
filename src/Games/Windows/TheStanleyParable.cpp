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

    //m_bWaitEnabled = 8264; // CCommandBuffer::AddText
    tickcount = 103; // CClientState::ProcessTick
    interval_per_tick = 73; // CClientState::ProcessTick
    HostState_OnClientConnected = 695; // CClientState::SetSignonState

    // server.dll

    GetAllServerClasses = 10; // CServerGameDLL (TODO)
    m_iClassName = 104; // CBaseEntity (TODO)
    S_m_vecAbsOrigin = 468; // CBaseEntity (TODO)
    S_m_angAbsRotation = 480; // CBaseEntity (TODO)
    m_iEFlags = 208; // CBaseEntity (TODO)
    m_flGravity = 792; // CBaseEntity (TODO)
    NUM_ENT_ENTRIES = 4096; // CBaseEntityList::CBaseEntityList (TODO)
    GetIServerEntity = 2; // CServerTools (TODO)
    m_EntPtrArray = 48; // CServerTools::GetIServerEntity (TODO)

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
const char* TheStanleyParable::Version()
{
    return "The Stanley Parable (5454)";
}
const char* TheStanleyParable::Process()
{
    return "stanley.exe";
}
