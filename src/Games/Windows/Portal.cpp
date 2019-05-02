#include "Portal.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

Portal::Portal()
{
    this->version = SourceGame_Portal;
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
