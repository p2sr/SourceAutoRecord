#include "Offsets/Portal 2 7054.hpp"

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

SIGSCAN_LINUX(PathMatch, "55 89 E5 57 56 53 83 EC ? 0F B6 45 ? 80 3D")
SIGSCAN_LINUX(GetChapterProgress, "55 89 E5 57 56 53 83 EC 2C 8B 7D 08 E8 ? ? ? ? 8B 10 C7")
