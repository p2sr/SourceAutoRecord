#include "Portal.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

Portal::Portal()
{
    this->version = SourceGame_Portal;
    Game::mapNames = {
        "testchmb_a_00",
        "testchmb_a_01",
        "testchmb_a_02",
        "testchmb_a_03",
        "testchmb_a_04",
        "testchmb_a_05",
        "testchmb_a_06",
        "testchmb_a_07",
        "testchmb_a_08",
        "testchmb_a_09",
        "testchmb_a_10",
        "testchmb_a_11",
        "testchmb_a_13",
        "testchmb_a_14",
        "testchmb_a_15",
        "escape_00",
        "escape_01",
        "escape_02",
    };
}
void Portal::LoadOffsets()
{
    HalfLife2::LoadOffsets();

    using namespace Offsets;

    // engine.dll

    Cbuf_AddText = 60; // CEngineClient::ClientCmd
    s_CommandBuffer = 71; // Cbuf_AddText
    AddText = 76; // Cbuf_AddText
    tickcount = 98; // CClientState::ProcessTick
    interval_per_tick = 68; // CClientState::ProcessTick
    HostState_OnClientConnected = 570; // CClientState::SetSignonState
    demoplayer = 115; // CClientState::Disconnect
    demorecorder = 128; // CClientState::Disconnect
    m_szLevelName = 34; // CEngineTool::GetCurrentMap
    //FireEventIntern = 12; // CGameEventManager::FireEventClientSide
    //ConPrintEvent = 262; // CGameEventManager::FireEventIntern
    AutoCompletionFunc = 66; // listdemo_CompletionFunc
    Key_SetBinding = 135; // unbind

    // server.dll

    UTIL_PlayerByIndex = 39; // CServerGameDLL::Think
    gpGlobals = 15; // UTIL_PlayerByIndex
    iNumPortalsPlaced = 4796; // CPortal_Player::IncrementPortalsPlaced
    m_EntPtrArray = 53; // CServerTools::GetIServerEntity

    // vguimatsurface.dll

    StartDrawing = 129; // CMatSystemSurface::PaintTraverseEx
    FinishDrawing = 650; // CMatSystemSurface::PaintTraverseEx
}
const char* Portal::Version()
{
    return "Portal (1910503)";
}
const char* Portal::ModDir()
{
    return "portal";
}
