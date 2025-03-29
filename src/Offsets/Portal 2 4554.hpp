#include "Offsets/Portal 2 5723.hpp"

/* anything not mentioned in this file could be wrong/most likely was not checked */

// Client
// stolen from Beginner's Guide
SIGSCAN_WINDOWS(MatrixBuildRotationAboutAxis, "55 8B EC 51 F3 0F 10 45 ? 0F 5A C0 F2 0F 59 05 ? ? ? ? 66 0F 5A C0 F3 0F 11 45 ? F3 0F 5A C0 E8 ? ? ? ? F2 0F 5A C0 F3 0F 11 45 ? F3 0F 10 45 ? 0F 5A C0 E8 ? ? ? ? 8B 45 ? F3 0F 10 60")

// stolen from Infra
SIGSCAN_WINDOWS(DrawTranslucentRenderables, "55 8B EC 81 EC 80 00 00 00 53 56 8B F1")

// same as something else
OFFSET_WINDOWS(HostState_OnClientConnected, 695)

OFFSET_WINDOWS(gamerules, 5)

//stolen
OFFSET_WINDOWS(tickcount, 103)
OFFSET_WINDOWS(interval_per_tick, 73)

OFFSET_WINDOWS(GetLocalClient, 138)

// CEngineAPI
// all good

// CEngineClient
// this game version doesn't have GetDemoHeaderInfo at index 36, so everything past 35 is -1 compared to latest
OFFSET_WINDOWS(GetLevelNameShort, 52)
OFFSET_WINDOWS(DebugDrawPhysCollide, 74)
OFFSET_WINDOWS(IsPaused, 85)
OFFSET_WINDOWS(ExecuteClientCmd, 103)
OFFSET_WINDOWS(GetSaveDirName, 123)
OFFSET_WINDOWS(GetActiveSplitScreenPlayerSlot, 126)
OFFSET_WINDOWS(GetSteamAPIContext, 176)

// CHLClient
//OFFSET_DEFAULT(GetAllClasses, 8, 8) // verified
//OFFSET_DEFAULT(HudProcessInput, 10, 10) // verified
//OFFSET_WINDOWS(HudUpdate, 11) // found, if needed
//OFFSET_DEFAULT(IN_ActivateMouse, 15, 15) // verified
//OFFSET_DEFAULT(IN_DeactivateMouse, 16, 16) // verified
OFFSET_WINDOWS(ApplyMouse, 53) // found
OFFSET_EMPTY(SteamControllerMove) // THIS DOES NOT EXIST IN 2011!!!! (because the Steam Controller didn't exist in 2011)
//OFFSET_WINDOWS(JoyStickApplyMovement, 60) // found, if needed
//OFFSET_DEFAULT(LevelInitPreEntity, 5, 5) // verified

// CVEngineServer
// all good

// CHudChat/CBaseHudChat
//OFFSET_WINDOWS(MsgFunc_SayText2, 27) // unused, see comment in latest P2 offsets file
//OFFSET_WINDOWS(GetTextColorForClient, 32) // unused but ye

// vgui stuff, pretty sure I just stole these from another version, no idea if they are correct,
// but the game doesn't crash.
OFFSET_WINDOWS(DrawColoredCircle, 158)
OFFSET_WINDOWS(DrawColoredText, 159)
OFFSET_WINDOWS(DrawTextLen, 162)

// CMaterialSystem - more stuff could be wrong, dunno, don't care to check
OFFSET_WINDOWS(UncacheUnusedMaterials, 76) // confirmed
OFFSET_WINDOWS(CreateMaterial, 80) // confirmed
OFFSET_WINDOWS(FindMaterial, 81) // confirmed
OFFSET_WINDOWS(CreateProceduralTexture, 89) // confirmed
OFFSET_WINDOWS(GetRenderContext, 111) // found
OFFSET_WINDOWS(RemoveMaterial, 153) // found

// CEngineTool
//OFFSET_DEFAULT(GetCurrentMap, 25, 26) // good
//OFFSET_DEFAULT(HostFrameTime, 39, 40) // good
//OFFSET_DEFAULT(HostTick, 41, 42) // good
//OFFSET_DEFAULT(ServerTick, 45, 46) // seems good
//OFFSET_DEFAULT(ClientTime, 47, 48) // seems good
//OFFSET_DEFAULT(ClientTick, 49, 50) // seems good
OFFSET_WINDOWS(PrecacheModel, 62)

// CServerGameDLL
// didn't check the ShouldDraw offset, but the vtable offsets defined in 9568 are good

// Server
SIGSCAN_WINDOWS(ViewPunch, "55 8B EC A1 ? ? ? ? 83 EC 0C 83 78 ? 00 56 8B F1") // this works for 9568 too, maybe shorten that pattern/adjust to match this?

// CBasePlayer
OFFSET_WINDOWS(m_pShadowStand, 3156) // found
OFFSET_WINDOWS(m_pShadowCrouch, 3160) // found

OFFSET_WINDOWS(viewangles, 19112) // found

// CBaseEntity
OFFSET_WINDOWS(AcceptInput, 39) // found
OFFSET_WINDOWS(IsPlayer, 84) // found

// CPortal_Player
//OFFSET_WINDOWS(GetPaintPower, 2) // this seems to be fine as is
OFFSET_WINDOWS(PlayerRunCommand, 452) // found
OFFSET_WINDOWS(UseSpeedPower, 508) // found

// CSteam3Client
OFFSET_WINDOWS(OnGameOverlayActivated, 92) // found

// IPhysicsObject
// everything (should) be fine.

// Steam API
SIGSCAN_EMPTY(interfaceMgrSig)
OFFSET_EMPTY(interfaceMgrOff)

// CDemoRecorder
// all same as current P2

// CDemoPlayer
OFFSET_WINDOWS(SkipToTick, 14) // found
// rest are good

// stolen
SIGSCAN_WINDOWS(GetHudSig, "55 8B EC 8B 45 ? 83 F8 FF 75 ? 8B 0D ? ? ? ? 8B 01 8B 90 ? ? ? ? FF D2 C1 E0 07")
SIGSCAN_WINDOWS(FindElementSig, "55 8B EC 53 8B 5D ? 56 57 8B F1 33 FF")

OFFSET_WINDOWS(m_szLevelName, 36)
OFFSET_WINDOWS(m_bLoadGame, 440)

SIGSCAN_WINDOWS(readConsoleCommandInjectSig, "8B 45 F4 50 68 ? 04 00 00 68 ? ? ? ? 8D 4D 90 E8 ? ? ? ? 8D 4F 04 E8") // needed to mask one byte

OFFSET_WINDOWS(net_time, 21) // found

OFFSET_WINDOWS(PerUserInput_tSize, 212) // found
OFFSET_WINDOWS(m_pCommands, 224) // found
// multiplayer_backup is good

// check/refind these
OFFSET_WINDOWS(m_pSurfaceData, 4088) // found but unsure
OFFSET_WINDOWS(S_m_surfaceFriction, 4092) // found
OFFSET_WINDOWS(C_m_surfaceFriction, 5532) // found