#include "Offsets/Portal 2 8151.hpp"
// Aperture Tag

// FIXME: OnGameOverlayActivated

OFFSET_DEFAULT(m_pCommands, 228, 228)
OFFSET_DEFAULT(RemoveMaterial, 155, 156)
OFFSET_LINUX(g_movieInfo, 11)
OFFSET_LINUX(SND_IsRecording, 10)
OFFSET_LINUX(gamerules, 7)

OFFSET_LINUX(say_callback_insn, 57)
OFFSET_LINUX(Host_Say_insn, 0x346)
OFFSET_LINUX(portalsThruPortals, 409)
OFFSET_LINUX(portalsThruPortalsValOn, 0x85)

// Renderer
SIGSCAN_LINUX(SND_RecordBuffer, "55 89 E5 57 56 53 83 EC 2C E8 ? ? ? ? 84 C0 75 0E 8D 65 F4 5B 5E 5F 5D C3")

// Client
SIGSCAN_LINUX(DrawTranslucentRenderables, "55 89 E5 57 56 8D 55 ? 53 81 EC ? ? ? ? 0F B6 45 ? 89 14 24")
SIGSCAN_LINUX(DrawOpaqueRenderables, "55 89 E5 57 56 53 81 EC ? ? ? ? A1 ? ? ? ? 8B 5D ? 85 C0 0F 95 C0 84 C0 88 45 ? 74 ? 8B 35 ? ? ? ? E8 ? ? ? ? 39 C6 0F 84 ? ? ? ? A1 ? ? ? ? 8B 40")
SIGSCAN_LINUX(UTIL_Portal_Color, "55 89 E5 83 EC 28 89 75 ? 8B 75 ? 89 5D ? 8B 5D ? 89 7D ? 8B 7D ? 83 FE 00")
SIGSCAN_LINUX(UTIL_Portal_Color_Particles, "55 89 E5 56 53 83 EC 10 A1 ? ? ? ? 8B 5D ? 8B 75 ? 8B 10 89 04 24 FF 92 ? ? ? ? 84 C0")

// Engine
SIGSCAN_LINUX(Host_AccumulateTime, "55 89 E5 83 EC ? F3 0F 10 45 ? A1")
SIGSCAN_LINUX(_Host_RunFrame_Render, "55 89 E5 56 53 83 EC ? 8B 0D ? ? ? ? 85 C9 0F 95 C0")
SIGSCAN_LINUX(readCustomDataInjectSig, "8B 95 94 FE FF FF 8D 4D D8 8D 45 D4 89 4C 24 08 89 44 24 04 89 14 24 E8")
OFFSET_LINUX(readCustomDataInjectOff, 24)
SIGSCAN_LINUX(readConsoleCommandInjectSig, "8B 45 ? 8D 75 ? C7 44 24 ? ? ? ? ? C7 44 24 ? ? ? ? ? 89 34 24 89 44 24 ? E8 ? ? ? ? 8B 8D")
OFFSET_LINUX(readConsoleCommandInjectOff, 44)
SIGSCAN_LINUX(Cmd_ExecuteCommand, "55 89 E5 57 56 53 83 EC 1C 8B 5D ? 83 3B 00")
SIGSCAN_LINUX(InsertCommand, "55 89 E5 83 EC 38 89 75 ? 8B 75 ? 89 5D ? 8B 5D ? 89 7D ? 8B 7D ? 81 FE FE 01 00 00")

// EngineDemoPlayer
SIGSCAN_LINUX(InterpolateDemoCommand, "55 89 E5 57 56 53 83 EC ? 8B 45 ? 8B 55 ? 8B 7D ? 8B 98")

// Matchmaking
SIGSCAN_LINUX(UpdateLeaderboardData, "55 89 E5 57 56 53 83 EC 3C 8B 55 ? 8B 02 89 14 24 FF 50 ? C7 04 24")

// Server
SIGSCAN_LINUX(GlobalEntity_GetIndex, "55 89 E5 56 53 8D 45 ? 83 EC ? 8B 55 ? C7 44 24")
SIGSCAN_LINUX(GlobalEntity_SetFlags, "80 3D ? ? ? ? ? 55 89 E5 8B 45 ? 75 ? 85 C0 78 ? 3B 05 ? ? ? ? 7D ? 8B 15 ? ? ? ? 8D 04 40 8D 04 82 8B 55 ? 89 50 ? 5D C3 90 80 3D")
SIGSCAN_LINUX(Host_Say, "55 89 E5 57 56 53 81 EC 7C 02 00 00 8B 5D")
SIGSCAN_LINUX(TraceFirePortal, "55 89 E5 57 56 8D 7D ? 53 81 EC ? ? ? ? 0F B6 45")
SIGSCAN_LINUX(FindPortal, "55 89 E5 57 56 53 83 EC ? 0F B6 45 ? 0F B6 55 ? 88 45")
SIGSCAN_LINUX(ViewPunch, "55 89 E5 83 EC ? A1 ? ? ? ? 89 5D ? 89 75 ? 8B 5D ? 8B 75 ? 8B 40 ? 85 C0 74 ? 8B 5D")
SIGSCAN_LINUX(UTIL_FindClosestPassableSpace, "55 89 E5 57 56 53 81 EC ? ? ? ? C6 85 ? ? ? ? ? 8B 45 ? C6 85")
SIGSCAN_LINUX(FindClosestPassableSpace, "55 B8 ? ? ? ? 89 E5 57 56 53 81 EC ? ? ? ? 8B 15")
SIGSCAN_LINUX(CheckStuck_FloatTime, "E8 ? ? ? ? 8B 43 ? DD 9D")
SIGSCAN_LINUX(aircontrol_fling_speedSig, "0F 2E 05 ? ? ? ? 0F 86 ? ? ? ? 0F 2E 15")

// Steam API
SIGSCAN_WINDOWS(interfaceMgrSig, "A3 ? ? ? ? 3B C3 0F 84")
OFFSET_WINDOWS(interfaceMgrOff, 1)
