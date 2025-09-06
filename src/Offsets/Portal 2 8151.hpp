#include "Offsets/Portal 2 9568.hpp"
// Portal Stories: Mel, Reloaded

OFFSET_LINUX(tickcount, 74)
OFFSET_LINUX(interval_per_tick, 82)
OFFSET_LINUX(demoplayer, 92)
OFFSET_LINUX(demorecorder, 105)
OFFSET_LINUX(m_szLevelName, 64)
OFFSET_LINUX(gpGlobals, 12)
OFFSET_LINUX(Cbuf_AddText, 45)
OFFSET_LINUX(s_CommandBuffer, 71)
OFFSET_LINUX(eng, 7)
OFFSET_LINUX(hoststate, 9)
OFFSET_LINUX(m_EntPtrArray, 48)
OFFSET_LINUX(g_pClientMode, 25)
OFFSET_LINUX(net_time, 24)
OFFSET_LINUX(videomode, 183)
OFFSET_LINUX(GetClientStateFunction, 11)
OFFSET_LINUX(GetLocalClient, 92)
OFFSET_LINUX(HostState_OnClientConnected, 1523)
OFFSET_LINUX(FireEventIntern, 36)
OFFSET_LINUX(ConPrintEvent, 254)
OFFSET_LINUX(Key_SetBinding, 60)
OFFSET_LINUX(VideoMode_Create, 104)
OFFSET_LINUX(AirMove_Offset1, 14)
OFFSET_LINUX(AirMove_Offset2, 12)
OFFSET_LINUX(UTIL_PlayerByIndex, 61)
OFFSET_LINUX(GetClientMode, 11)
OFFSET_LINUX(GetHud, 104)
OFFSET_LINUX(FindElement, 120)
OFFSET_LINUX(StartDrawing, 692)
OFFSET_LINUX(FinishDrawing, 627)
OFFSET_DEFAULT(OnGameOverlayActivated, 144, 152)
OFFSET_LINUX(FontManager, 11)
OFFSET_LINUX(snd_linear_count, 57)
OFFSET_LINUX(snd_p, 65)
OFFSET_LINUX(snd_vol, 71)
OFFSET_LINUX(CBaseEntity_Create, 163)
OFFSET_LINUX(g_pTextureManager, 10)
OFFSET_LINUX(gamerules, 9)
OFFSET_LINUX(OpenRadialMenuCommand, 12)

OFFSET_LINUX(host_frametime, 70)

OFFSET_LINUX(say_callback_insn, 88)
OFFSET_LINUX(Host_Say_insn, 0x37A)
OFFSET_LINUX(portalsThruPortals, 462)

// clang-format off

// Pathmatch
SIGSCAN_LINUX(PathMatch, "55 89 E5 57 56 53 83 EC 2C 8B 45 10 80 3D A0 ? ? ? ? 89 45 E4 0F 84 ? ? ? ? 8B 45 0C 8B 15 ? ? ? ? C7 00 00 00 00 00")

// Renderer
SIGSCAN_LINUX(SND_RecordBuffer, "55 89 E5 57 56 53 83 EC 3C 65 A1 ? ? ? ? 89 45 E4 31 C0 E8 ? ? ? ? 84 C0 75 1B")

// Client
SIGSCAN_LINUX(MatrixBuildRotationAboutAxis, "55 89 E5 56 53 8D 45 ? 8D 55 ? 83 EC 20")
SIGSCAN_LINUX(DrawTranslucentRenderables, "55 89 E5 57 56 53 81 EC DC 00 00 00 8B 45 08 8B 5D 0C 89 C7 89 45 84 8B 45 10 89 85 4C FF FF FF")
SIGSCAN_LINUX(DrawPortal, "55 89 E5 57 56 53 83 EC 3C A1 ? ? ? ? 8B 5D ? 8B 10 89 04 24 FF 52 ? A8 02")
SIGSCAN_LINUX(DrawPortalSpBranch, "0F 85 ? ? ? ? 0F B6 83 ? ? ? ? 8B 34 85")
OFFSET_LINUX(DrawPortalSpBranchOff, 0x15)
SIGSCAN_LINUX(DrawPortalGhost, "A1 ? ? ? ? 8B 40 ? 85 C0 74 ? 55 89 E5 57 56 53 83 EC 4C")
SIGSCAN_LINUX(DrawPortalGhostSpBranch, "0F 84 ? ? ? ? 8B 03 89 1C 24 FF 90 ? ? ? ? 80 BB ? ? ? ? 01")
SIGSCAN_LINUX(DrawOpaqueRenderables, "55 89 E5 57 56 53 81 EC 8C 00 00 00 8B 45 0C 8B 5D 08 89 45 8C 8B 45 14 89 45 90 65 A1 14 00 00 00")
SIGSCAN_LINUX(AddShadowToReceiver, "55 89 E5 57 56 53 83 EC ? 8B 45 ? 8B 4D ? 8B 7D ? 89 45 ? 0F B7 C0")
SIGSCAN_LINUX(UTIL_Portal_Color, "55 89 E5 56 53 83 EC 10 8B 75 ? 8B 5D ? 85 F6 0F 84")
SIGSCAN_LINUX(UTIL_Portal_Color_Particles, "55 89 E5 53 83 EC 14 A1 ? ? ? ? 8B 5D ? 8B 10 89 04 24 FF 92 ? ? ? ? 84 C0 75 ? 83 7D ? 01")
SIGSCAN_LINUX(GetChapterProgress, "55 89 E5 57 56 53 83 EC 2C 8B 5D 08 E8 ? ? ? ? 8B 10")

// Engine
SIGSCAN_LINUX(Host_AccumulateTime, "55 89 E5 83 EC 28 F3 0F 10 05 ? ? ? ? A1 ? ? ? ? F3 0F 58 45 08 F3 0F 11 05 ? ? ? ? 8B 10 89 04 24 FF 52 24")
SIGSCAN_LINUX(_Host_RunFrame_Render, "55 89 E5 57 56 53 83 EC 2C 8B 35 ? ? ? ? 85 F6 0F 95 C0 89 C6 0F 85 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? 80 3D ? ? ? ? 00 8B 78 30")
SIGSCAN_LINUX(readCustomDataInjectSig, "89 44 24 08 8D 85 B0 FE FF FF 89 44 24 04 8B 85 8C FE FF FF 89 04 24 E8")
OFFSET_LINUX(readCustomDataInjectOff, 24)
SIGSCAN_LINUX(readConsoleCommandInjectSig, "89 44 24 0C 8D 85 C0 FE FF FF 89 04 24 E8 ? ? ? ? 8B 85 8C FE FF FF 89 04 24 E8")
OFFSET_LINUX(readConsoleCommandInjectOff, 28)
SIGSCAN_LINUX(Cmd_ExecuteCommand, "55 89 E5 57 56 53 83 EC 2C 8B 75 ? 83 3E 00")
SIGSCAN_LINUX(InsertCommand, "55 89 E5 57 56 53 83 EC 1C 8B 75 ? 8B 5D ? 81 FE FE 01 00 00")

// EngineDemoPlayer
SIGSCAN_LINUX(InterpolateDemoCommand, "55 31 C9 89 E5 57 56 53 83 EC 3C 89 4D F0 8B 45 08 8B 4D 14 8B 80 B0 05 00 00 89 45 B8 8B 45 14 83 C0 04 89 45 D0")

// MaterialSystem
SIGSCAN_LINUX(KeyValues_SetString, "55 89 E5 53 83 EC ? 8B 45 ? C7 44 24 ? ? ? ? ? 8B 5D ? 89 44 24 ? 8B 45 ? 89 04 24 E8 ? ? ? ? 85 C0 74 ? 89 5D")

// Server
SIGSCAN_LINUX(GlobalEntity_GetIndex, "55 89 E5 53 8D 45 F6 83 EC 24 8B 55 08 C7 44 24 04 ? ? ? ? 89 04 24 89 54 24 08")
SIGSCAN_LINUX(GlobalEntity_SetFlags, "80 3D ? ? ? ? 00 55 89 E5 8B 45 08 75 1E 85 C0 78 1A 3B 05 ? ? ? ? 7D 12 8B 15")
SIGSCAN_LINUX(Host_Say, "55 89 E5 57 56 53 81 EC 8C 02 00 00 8B 45 ? 8B 5D")
SIGSCAN_LINUX(TraceFirePortal, "55 89 E5 57 56 8D BD F4 F8 FF FF 53 81 EC 3C 07 00 00 8B 45 14 C7 44 24 08 00 00 00 00 89 3C 24 8B 5D 0C")
SIGSCAN_LINUX(FindPortal, "55 89 E5 57 56 53 83 EC 2C 8B 45 08 8B 7D 0C 8B 4D 10 89 45 D8 0F B6 C0 8D 04 80 89 7D E0 C1 E0 02 89 4D DC")
SIGSCAN_LINUX(ViewPunch, "55 89 E5 53 83 EC 24 A1 ? ? ? ? 8B 5D 08 8B 40 30 85 C0 74 0A 83 C4 24 5B 5D C3 8D 74 26 00 8B 03 89 1C 24 FF 90 04 05 00 00 84 C0 75 E7 8B 45 0C")
SIGSCAN_LINUX(UTIL_FindClosestPassableSpace, "55 89 E5 57 56 53 81 EC BC 02 00 00 C6 85 7C FE FF FF 00 8B 45 0C C6 85 7D FE FF FF 01 8B 4D 08 C7 85 78 FE FF FF 00 00 00 00")
SIGSCAN_LINUX(FindClosestPassableSpace, "8B 15 ? ? ? ? B8 01 00 00 00 8B 52 30 85 D2 0F 84 ? ? ? ? 55 89 E5 57 56 53 81 EC 7C 02 00 00 8B 55 08 8B 0D ? ? ? ? 8B 92 E4 00 00 00")
SIGSCAN_LINUX(UTIL_GetCommandClientIndex, "A1 ? ? ? ? 55 89 E5 5D 83 C0 01 C3")
SIGSCAN_LINUX(CheckStuck_FloatTime, "E8 ? ? ? ? 8B 43 04 DD 9D ? ? ? ? F2 0F 10 B5 ? ? ? ? 8B 50 24 66 0F 14 F6 66 0F 5A CE 85 D2")
SIGSCAN_DEFAULT(aircontrol_fling_speedSig, "0F 2F 25 ? ? ? ? 0F 28 F0",
                                           "0F 2E 05 ? ? ? ? 0F 86 ? ? ? ? 0F 2E 25")

// clang-format on
