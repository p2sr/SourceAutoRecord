#include "PortalStoriesMel.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

PortalStoriesMel::PortalStoriesMel()
{
    this->version = SourceGame_PortalStoriesMel;
    Game::mapNames = {
        "sp_a1_tramride",
        "sp_a1_mel_intro",
        "sp_a1_lift",
        "sp_a1_garden",
        "sp_a2_garden_de",
        "sp_a2_underbounce",
        "sp_a2_once_upon",
        "sp_a2_past_power",
        "sp_a2_ramp",
        "sp_a2_firestorm",
        "sp_a3_junkyard",
        "sp_a3_concepts",
        "sp_a3_paint_fling",
        "sp_a3_faith_plate",
        "sp_a3_transition",
        "sp_a4_overgrown",
        "sp_a4_tb_over_goo",
        "sp_a4_two_of_a_kind",
        "sp_a4_destroyed",
        "sp_a4_factory",
        "sp_a4_core_access",
        "sp_a4_finale"
    };
}
void PortalStoriesMel::LoadOffsets()
{
    Portal2::LoadOffsets();

    using namespace Offsets;

    // client.dll

    m_pCommands = 228; // CInput::DecodeUserCmdFromBuffer
    GetLocalClient = 85; // CEngineClient::SetViewAngles
    net_time = 28; // CDemoRecorder::GetRecordingTick
    tickcount = 73; // CClientState::ProcessTick
    interval_per_tick = 81; // CClientState::ProcessTick
    HostState_OnClientConnected = 735; // CClientState::SetSignonState
    demoplayer = 93; // CClientState::Disconnect
    demorecorder = 106; // CClientState::Disconnect
    m_szLevelName = 72; // CEngineTool::GetCurrentMap
    GetClientMode = 12; // CHLClient::HudProcessInput
    StartDrawing = 193; // CMatSystemSurface::PaintTraverseEx
    FinishDrawing = 590; // CMatSystemSurface::PaintTraverseEx
    VideoMode_Create = 103; // CEngineAPI::Init
}
const char* PortalStoriesMel::Version()
{
    return "Portal Stories: Mel (5723)";
}
const char* PortalStoriesMel::ModDir()
{
    return "portal_stories";
}
