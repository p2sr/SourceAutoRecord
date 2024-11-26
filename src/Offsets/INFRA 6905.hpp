#include "Offsets/Portal 2 7054.hpp"

// CEngineClient
OFFSET_WINDOWS(GetActiveSplitScreenPlayerSlot, 129)
OFFSET_WINDOWS(GetSteamAPIContext, 179)
OFFSET_WINDOWS(host_frametime, 87);

// CHLClient
OFFSET_WINDOWS(GetAllClasses, 9)
OFFSET_WINDOWS(HudProcessInput, 11)
OFFSET_WINDOWS(HudUpdate, 12)
OFFSET_WINDOWS(IN_ActivateMouse, 16)
OFFSET_WINDOWS(IN_DeactivateMouse, 17)
OFFSET_WINDOWS(ApplyMouse, 54)
OFFSET_WINDOWS(ApplyMouse_Mid, 0x451)
OFFSET_WINDOWS(JoyStickApplyMovement, 61)

// CInputSystem
OFFSET_WINDOWS(KeyDown, 401)
OFFSET_WINDOWS(KeyUp, 380)

// CInput
OFFSET_WINDOWS(DecodeUserCmdFromBuffer, 6)
OFFSET_WINDOWS(SteamControllerMove, 52)

// ConVar
OFFSET_WINDOWS(InternalSetValue, 14)
OFFSET_WINDOWS(InternalSetFloatValue, 15)
OFFSET_WINDOWS(InternalSetIntValue, 16)
OFFSET_WINDOWS(Create, 29)

// CMatSystemSurface
OFFSET_WINDOWS(PaintTraverseEx, 118)
OFFSET_WINDOWS(GetKernedCharWidth, 148)
OFFSET_WINDOWS(GetFontName, 133)

// CGameMovement
OFFSET_WINDOWS(AirMove, 24)
OFFSET_WINDOWS(FinishGravity, 33)
OFFSET_WINDOWS(CheckJumpButton, 35)
OFFSET_WINDOWS(TryPlayerMove, 38)

// CClientState
OFFSET_WINDOWS(viewangles, 35424)

// CBasePlayer
OFFSET_WINDOWS(m_pShadowStand, 3200)
OFFSET_WINDOWS(m_pShadowCrouch, 3204)
OFFSET_WINDOWS(S_m_surfaceFriction, 4156)
OFFSET_WINDOWS(C_m_surfaceFriction, 5652)

// CPortal_Player
OFFSET_WINDOWS(PlayerRunCommand, 452)

// CServerGameDLL
OFFSET_WINDOWS(ApplyGameSettings, 39)
OFFSET_WINDOWS(Think, 32)
OFFSET_WINDOWS(IsRestoring, 25)
OFFSET_WINDOWS(ShouldDraw, 12)

// CGameEventManager
OFFSET_WINDOWS(ConPrintEvent, 351)

// CVEngineServer
OFFSET_WINDOWS(ClientCommand, 40)
OFFSET_WINDOWS(ClientCommandKeyValues, 137)
OFFSET_WINDOWS(ChatPrintf, 23)

// CMaterialSystem
OFFSET_WINDOWS(UncacheUnusedMaterials, 76)
OFFSET_WINDOWS(CreateMaterial, 80)
OFFSET_WINDOWS(FindMaterial, 81)
OFFSET_WINDOWS(CreateProceduralTexture, 90)
OFFSET_WINDOWS(GetRenderContext, 112)

// Others
OFFSET_WINDOWS(tickcount, 104)
OFFSET_WINDOWS(interval_per_tick, 74)
OFFSET_WINDOWS(HostState_OnClientConnected, 724)
OFFSET_WINDOWS(NUM_ENT_ENTRIES, 16384)
OFFSET_WINDOWS(m_pCommands, 380)
OFFSET_WINDOWS(CUserCmdSize, 116)
OFFSET_WINDOWS(PerUserInput_tSize, 388)
OFFSET_WINDOWS(GetLocalClient, 138)
OFFSET_WINDOWS(net_time, 21)
OFFSET_WINDOWS(snd_linear_count, 69)
OFFSET_WINDOWS(snd_p, 97)
OFFSET_WINDOWS(snd_vol, 107)

// clang-format off

// Renderer
SIGSCAN_WINDOWS(SND_RecordBuffer, "55 8B EC 80 3D ? ? ? ? 00 56")


// Client
SIGSCAN_WINDOWS(DrawTranslucentRenderables, "55 8B EC 81 EC 80 00 00 00 53 56 8B F1")
SIGSCAN_WINDOWS(DrawPortalGhost, "")
SIGSCAN_WINDOWS(DrawOpaqueRenderables, "55 8B EC 83 EC 54 A1 ? ? ? ? 83 7D ? 00")
SIGSCAN_WINDOWS(GetHudSig, "55 8B EC 8B 45 ? 83 F8 FF 75 ? 8B 0D ? ? ? ? 8B 01 8B 90 ? ? ? ? FF D2 69 C0 84 00 00 00") // usage of FindElement -> probably previous function call
SIGSCAN_WINDOWS(FindElementSig, "55 8B EC 53 8B 5D ? 56 57 8B F1 33 FF") // "[%d] Could not find Hud Element: %s\n" xref
SIGSCAN_WINDOWS(GetChapterProgress, "")


// Engine
SIGSCAN_WINDOWS(Host_AccumulateTime, "55 8B EC F3 0F 10 05 ? ? ? ? F3 0F 58 45")
SIGSCAN_WINDOWS(readCustomDataInjectSig, "8D 4D E8 51 8D 55 BC 52 8D 4F 04 E8 ? ? ? ? 8B 4D BC 83 F9 FF")
OFFSET_WINDOWS(readCustomDataInjectOff, 12)
SIGSCAN_WINDOWS(readConsoleCommandInjectSig, "8B 55 F4 52 68 13 05 00 00 68 ? ? ? ? 8D 4D 90 E8 ? ? ? ? 8D 4F 04 E8")
OFFSET_WINDOWS(readConsoleCommandInjectOff, 26)
SIGSCAN_WINDOWS(Cmd_ExecuteCommand, "55 8B EC 57 8B 7D ? 8B 07 85 C0")
SIGSCAN_WINDOWS(InsertCommand, "55 8B EC 56 57 8B 7D ? 8B F1 81 FF FF 01 00 00")


// Server
SIGSCAN_WINDOWS(ViewPunch, "55 8B EC A1 ? ? ? ? 8B 50 ? 83 EC 0C 56 8B F1")
SIGSCAN_WINDOWS(FindClosestPassableSpace, "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B ? 89 6C 24 ? 8B EC A1 ? ? ? ? 8B 50 ? 81 EC 88 02 00 00")
SIGSCAN_WINDOWS(CheckStuck_FloatTime, "FF 15 ? ? ? ? D9 5D ? 8B 4E")


// Steam API
SIGSCAN_WINDOWS(interfaceMgrSig, "83 3D ? ? ? ? 00 74 ? B0 01")
OFFSET_WINDOWS(interfaceMgrOff, 2)

// clang-format on
