#include "Offsets/Portal 2 7054.hpp"
// Thinking With Time Machine

OFFSET_DEFAULT(RemoveMaterial, 156, 157)

OFFSET_LINUX(tickcount, 73)
OFFSET_LINUX(interval_per_tick, 81)
OFFSET_LINUX(HostState_OnClientConnected, 735)
OFFSET_LINUX(demoplayer, 93)
OFFSET_LINUX(demorecorder, 106)
OFFSET_LINUX(m_szLevelName, 72)
OFFSET_LINUX(GetClientMode, 12)
OFFSET_LINUX(StartDrawing, 193)
OFFSET_LINUX(FinishDrawing, 590)
OFFSET_LINUX(GetLocalClient, 85)
OFFSET_LINUX(net_time, 28)
OFFSET_LINUX(PerUserInput_tSize, 344)
OFFSET_LINUX(VideoMode_Create, 103)
OFFSET_LINUX(snd_linear_count, 33)
OFFSET_LINUX(snd_p, 72)
OFFSET_LINUX(snd_vol, 80)

// Pathmatch
SIGSCAN_LINUX(PathMatch, "55 89 E5 57 56 53 83 EC ? 0F B6 45 ? 80 3D")

// Client
SIGSCAN_LINUX(DrawPortal, "55 89 E5 83 EC 58 A1 ? ? ? ? 89 5D ? 89 75 ? 8B 5D ? 89 7D ? 8B 75 ? 8B 10")
SIGSCAN_LINUX(DrawPortalSpBranch, "0F 85 ? ? ? ? 0F B6 83 ? ? ? ? 8B 3C 85 ? ? ? ? A1 ? ? ? ? 89 04 24 E8 ? ? ? ? 84 C0 74")
OFFSET_LINUX(DrawPortalSpBranchOff, 0x15)
SIGSCAN_LINUX(DrawPortalGhost, "55 89 E5 57 56 53 83 EC 5C A1 ? ? ? ? 8B 40")
SIGSCAN_LINUX(DrawPortalGhostSpBranch, "0F 84 ? ? ? ? FF 90 ? ? ? ? 80 BB ? ? ? ? 01")
SIGSCAN_LINUX(GetChapterProgress, "55 89 E5 57 56 53 83 EC 2C 8B 7D 08 E8 ? ? ? ? 8B 10 C7")
