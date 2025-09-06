#include "Offsets/Portal 2 7054.hpp"
// Beginner's Guide

// CClientState
OFFSET_DEFAULT(viewangles, 19112, 19084)

// CBasePlayer
OFFSET_WINDOWS(m_pShadowStand, 3164)
OFFSET_WINDOWS(m_pShadowCrouch, 3168)

// CMatSystemSurface
OFFSET_WINDOWS(DrawColoredCircle, 158)
OFFSET_WINDOWS(DrawColoredText, 159)
OFFSET_WINDOWS(DrawTextLen, 162)
OFFSET_LINUX(FinishDrawing, 619)

// CInputSystem
OFFSET_WINDOWS(GetRawMouseAccumulators, 52)
OFFSET_DEFAULT(KeyDown, 255, 337)
OFFSET_DEFAULT(KeyUp, 234, 384)

// CInput
OFFSET_WINDOWS(SteamControllerMove, 51)
OFFSET_WINDOWS(ApplyMouse, 53)
OFFSET_DEFAULT(JoyStickApplyMovement, 60, 64)

// CCommandBuffer
OFFSET_DEFAULT(m_bWaitEnabled, 8264, 8264)

// CMaterialSystem
OFFSET_WINDOWS(CreateMaterial, 80)
// OFFSET_WINDOWS(FindMaterial, 81)
OFFSET_WINDOWS(CreateProceduralTexture, 90)
OFFSET_WINDOWS(GetRenderContext, 112)

// IVideoMode
OFFSET_WINDOWS(GetModeWidth, 15)
OFFSET_WINDOWS(GetModeHeight, 16)

// Others
OFFSET_WINDOWS(tickcount, 103)
OFFSET_WINDOWS(interval_per_tick, 73)
OFFSET_LINUX(m_szLevelName, 56)
OFFSET_DEFAULT(in_jump, 420, 210)
OFFSET_LINUX(Key_SetBinding, 59)
OFFSET_WINDOWS(HostState_OnClientConnected, 695)
OFFSET_WINDOWS(m_pCommands, 224)
OFFSET_WINDOWS(CUserCmdSize, 96)
OFFSET_WINDOWS(PerUserInput_tSize, 212)
OFFSET_WINDOWS(GetLocalClient, 138)
OFFSET_DEFAULT(VideoMode_Create, 90, 106)
OFFSET_LINUX(videomode, 183)
OFFSET_WINDOWS(UncacheUnusedMaterials, 76)

// clang-format off

// Pathmatch
SIGSCAN_LINUX(PathMatch, "55 89 E5 57 56 53 83 EC 2C 8B 45 ? 80 3D ? ? ? ? 00")


// Renderer
SIGSCAN_DEFAULT(SND_RecordBuffer, "55 8B EC 80 3D ? ? ? ? 00 56",
                                  "55 89 E5 57 56 53 83 EC 3C 65 A1 ? ? ? ? 89 45 ? 31 C0 80 3D ? ? ? ? 00")

// Client
SIGSCAN_WINDOWS(MatrixBuildRotationAboutAxis, "55 8B EC 51 F3 0F 10 45 ? 0F 5A C0 F2 0F 59 05 ? ? ? ? 66 0F 5A C0 F3 0F 11 45 ? F3 0F 5A C0 E8 ? ? ? ? F2 0F 5A C0 F3 0F 11 45 ? F3 0F 10 45 ? 0F 5A C0 E8 ? ? ? ? 8B 45 ? F3 0F 10 60")
SIGSCAN_DEFAULT(DrawTranslucentRenderables, "55 8B EC 81 EC 80 00 00 00 53 56 8B F1 8B 0D",
                                            "55 89 E5 57 56 53 81 EC DC 00 00 00 8B 45 ? 8B 5D")
SIGSCAN_DEFAULT(DrawOpaqueRenderables, "55 8B EC 83 EC 54 83 7D ? 00 A1",
                                       "55 89 E5 57 56 53 81 EC 8C 00 00 00 8B 45 ? 8B 5D ? 89 45 ? 8B 45 ? 89 45 ? 65 A1 ? ? ? ? 89 45 ? 31 C0 A1")
SIGSCAN_WINDOWS(DrawPortal, "")
SIGSCAN_WINDOWS(DrawPortalSpBranch, "")
OFFSET_WINDOWS(DrawPortalSpBranchOff, -1)
SIGSCAN_WINDOWS(DrawPortalGhost, "")
SIGSCAN_WINDOWS(DrawPortalGhostSpBranch, "")
SIGSCAN_LINUX(UTIL_Portal_Color, "")
SIGSCAN_LINUX(UTIL_Portal_Color_Particles, "")
SIGSCAN_WINDOWS(GetHudSig, "55 8B EC 8B 45 ? 83 F8 FF 75 ? 8B 0D ? ? ? ? 8B 01 8B 90 ? ? ? ? FF D2 C1 E0 07")
SIGSCAN_WINDOWS(FindElementSig, "55 8B EC 53 8B 5D ? 56 57 8B F1 33 FF")
SIGSCAN_DEFAULT(GetChapterProgress, "",
                                    "")

// Engine
SIGSCAN_LINUX(Host_AccumulateTime, "55 89 E5 83 EC 28 F3 0F 10 05 ? ? ? ? A1")
SIGSCAN_LINUX(_Host_RunFrame_Render, "55 89 E5 57 56 53 83 EC 2C 8B 35 ? ? ? ? 85 F6 0F 95 C0")
SIGSCAN_LINUX(Cmd_ExecuteCommand, "55 89 E5 57 56 53 83 EC 2C 8B 75 ? 83 3E 00")
SIGSCAN_LINUX(InsertCommand, "55 89 E5 57 56 53 83 EC 1C 8B 75 ? 8B 5D ? 81 FE FE 01 00 00")
SIGSCAN_LINUX(readCustomDataInjectSig, "")
SIGSCAN_LINUX(readConsoleCommandInjectSig, "")

// EngineDemoPlayer
SIGSCAN_LINUX(InterpolateDemoCommand, "")

// Server
SIGSCAN_LINUX(aircontrol_fling_speedSig, "")
SIGSCAN_LINUX(GlobalEntity_GetIndex, "")
SIGSCAN_LINUX(Host_Say, "")
SIGSCAN_LINUX(TraceFirePortal, "")
SIGSCAN_LINUX(FindPortal, "")
SIGSCAN_LINUX(ViewPunch, "")
SIGSCAN_LINUX(FindClosestPassableSpace, "")

// Matchmaking
SIGSCAN_LINUX(UpdateLeaderboardData, "")

// Steam API
SIGSCAN_WINDOWS(interfaceMgrSig, "83 3D ? ? ? ? 00 74 ? B0 01")
OFFSET_WINDOWS(interfaceMgrOff, 2)

// clang-format on
