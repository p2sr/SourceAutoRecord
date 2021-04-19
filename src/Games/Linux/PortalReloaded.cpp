#include "PortalReloaded.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

PortalReloaded::PortalReloaded()
{
    this->version = SourceGame_PortalReloaded;
    Game::mapNames = {
        "sp_a1_pr_map_001",
        "sp_a1_pr_map_002",
        "sp_a1_pr_map_003",
        "sp_a1_pr_map_004",
        "sp_a1_pr_map_005",
        "sp_a1_pr_map_006",
        "sp_a1_pr_map_007",
        "sp_a1_pr_map_008",
        "sp_a1_pr_map_009",
        "sp_a1_pr_map_010",
        "sp_a1_pr_map_011",
        "sp_a1_pr_map_012",
    };
}
void PortalReloaded::LoadOffsets()
{
    Portal2::LoadOffsets();

    using namespace Offsets;

    // client.dll

    m_pCommands = 236; // CInput::DecodeUserCmdFromBuffer
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
    snd_linear_count = 33; // SND_RecordBuffer
    snd_p = 72; // SND_RecordBuffer
    snd_vol = 80; // SND_RecordBuffer
}
const char* PortalReloaded::Version()
{
    return "Portal Reloaded";
}
const char* PortalReloaded::ModDir()
{
    return "portalreloaded";
}
