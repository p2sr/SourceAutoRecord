#ifndef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif


// Interface
OFFSET_DEFAULT(CreateInterfaceInternal, 5, 5)
OFFSET_DEFAULT(s_pInterfaceRegs, 6, 11)

// CCvar
OFFSET_DEFAULT(RegisterConCommand, 9, 9)
OFFSET_DEFAULT(UnregisterConCommand, 10, 10)
OFFSET_DEFAULT(FindCommandBase, 13, 13)
OFFSET_DEFAULT(InstallGlobalChangeCallback, 19, 19)
OFFSET_DEFAULT(RemoveGlobalChangeCallback, 20, 20)
OFFSET_DEFAULT(m_pConCommandList, 48, 48)
OFFSET_DEFAULT(m_DisplayFuncs, 24, 24)

// ConCommandBase
OFFSET_DEFAULT(IsCommand, 1, 2)

// CEngineClient
OFFSET_DEFAULT(GetScreenSize, 5, 5)
OFFSET_DEFAULT(ClientCmd, 7, 7)
OFFSET_DEFAULT(GetLocalPlayer, 12, 12)
OFFSET_DEFAULT(GetGameDirectory, 35, 35)
OFFSET_DEFAULT(GetViewAngles, 18, 18)
OFFSET_DEFAULT(SetViewAngles, 19, 19)
OFFSET_DEFAULT(GetMaxClients, 20, 20)
OFFSET_EMPTY(ServerCmdKeyValues)
OFFSET_DEFAULT(GetSaveDirName, 124, 124)
OFFSET_DEFAULT(ExecuteClientCmd, 104, 104)
OFFSET_DEFAULT(GetActiveSplitScreenPlayerSlot, 127, 127)
OFFSET_DEFAULT(GetSteamAPIContext, 177, 178)
OFFSET_DEFAULT(GetMouseDelta, -1, 161) // This method only exists on Linux
OFFSET_DEFAULT(IsPaused, 86, 86)
OFFSET_DEFAULT(DebugDrawPhysCollide, 75, 75)
OFFSET_DEFAULT(Con_IsVisible, 11, 11)
OFFSET_DEFAULT(GetLevelNameShort, 53, 53)
OFFSET_DEFAULT(GetLightForPoint, 1, 1)
OFFSET_DEFAULT(GetPlayerInfo, 8, 8)
OFFSET_DEFAULT(host_frametime_unbounded, 1, -1)

// CHLClient
OFFSET_DEFAULT(GetAllClasses, 8, 8)
OFFSET_DEFAULT(HudProcessInput, 10, 10)
OFFSET_EMPTY(HudUpdate)
OFFSET_DEFAULT(IN_ActivateMouse, 15, 15)
OFFSET_DEFAULT(IN_DeactivateMouse, 16, 16)
OFFSET_DEFAULT(ApplyMouse, 56, 56)
OFFSET_DEFAULT(ApplyMouse_Mid, 0x3E1, -1)
OFFSET_DEFAULT(SteamControllerMove, 58, 58)
OFFSET_EMPTY(JoyStickApplyMovement)
OFFSET_DEFAULT(LevelInitPreEntity, 5, 5)

// CInputSystem
OFFSET_EMPTY(KeyDown)
OFFSET_EMPTY(KeyUp)

// ClientModeShared
OFFSET_DEFAULT(CreateMove, 24, 25)
OFFSET_DEFAULT(OverrideView, 18, 19)

// ConVar
OFFSET_DEFAULT(Dtor, 9, 0)
OFFSET_DEFAULT(InternalSetValue, 12, 19)
OFFSET_DEFAULT(InternalSetFloatValue, 13, 20)
OFFSET_DEFAULT(InternalSetIntValue, 14, 21)
OFFSET_DEFAULT(Create, 27, 25)

// CMatSystemSurface
OFFSET_DEFAULT(DrawSetColor, 14, 13)
OFFSET_DEFAULT(DrawFilledRect, 15, 15)
OFFSET_DEFAULT(DrawColoredCircle, 159, 159)
OFFSET_DEFAULT(DrawLine, 18, 18)
OFFSET_DEFAULT(DrawSetTextFont, 22, 22)
OFFSET_DEFAULT(DrawSetTextColor, 23, 24)
OFFSET_DEFAULT(GetFontTall, 72, 72)
OFFSET_DEFAULT(PaintTraverseEx, 117, 117)
OFFSET_DEFAULT(DrawColoredText, 160, 160)
OFFSET_DEFAULT(DrawTextLen, 163, 163)
OFFSET_DEFAULT(GetKernedCharWidth, 147, 147)
OFFSET_DEFAULT(GetFontName, 132, 132)
OFFSET_DEFAULT(FontManager, 8, 9)
OFFSET_DEFAULT(DrawSetTextureFile, 35, 35)
OFFSET_DEFAULT(DrawSetTextureRGBA, 36, 36)
OFFSET_DEFAULT(DrawSetTexture, 37, 37)
OFFSET_DEFAULT(DrawGetTextureSize, 38, 38)
OFFSET_DEFAULT(DrawTexturedRect, 39, 39)
OFFSET_DEFAULT(IsTextureIDValid, 40, 40)
OFFSET_DEFAULT(CreateNewTextureID, 41, 41)

// CInputSystem
OFFSET_DEFAULT(StringToButtonCode, 31, 31)
OFFSET_DEFAULT(SleepUntilInput, 33, 33)
OFFSET_DEFAULT(GetRawMouseAccumulators, 50, 52)
OFFSET_DEFAULT(IsButtonDown, 14, 14)
OFFSET_DEFAULT(GetCursorPosition, 45, 45)
OFFSET_DEFAULT(SetCursorPosition, 38, 38)

// CInput
OFFSET_DEFAULT(GetButtonBits, 2, 2)
OFFSET_DEFAULT(ActivateMouse, 27, 27)
OFFSET_DEFAULT(DeactivateMouse, 28, 28)
OFFSET_DEFAULT(DecodeUserCmdFromBuffer, 7, 7)

// CGameMovement
OFFSET_DEFAULT(PlayerMove, 17, 16)
OFFSET_DEFAULT(AirAccelerate, 24, 23)
OFFSET_DEFAULT(AirMove, 25, 24)
OFFSET_DEFAULT(FinishGravity, 34, 35)
OFFSET_DEFAULT(CheckJumpButton, 36, 37)
OFFSET_DEFAULT(FullTossMove, -1, 38)
OFFSET_DEFAULT(TryPlayerMove, 39, 40)
OFFSET_DEFAULT(CheckStuck, 47, 48)
OFFSET_DEFAULT(StepMove, 70, 71)
OFFSET_DEFAULT(mv, 8, 8)
OFFSET_DEFAULT(player, 4, 4)
OFFSET_DEFAULT(ProcessMovement, 1, 2)
OFFSET_DEFAULT(GetPlayerViewOffset, 8, 9)

// CDemoRecorder
OFFSET_DEFAULT(GetRecordingTick, 1, 1)
OFFSET_DEFAULT(SetSignonState, 3, 3)
OFFSET_DEFAULT(StartRecording, 2, 2)
OFFSET_DEFAULT(StopRecording, 7, 7)
OFFSET_DEFAULT(RecordCustomData, 14, 14)
OFFSET_DEFAULT(RecordCommand, 8, 8)
OFFSET_DEFAULT(m_szDemoBaseName, 1344, 1344)
OFFSET_DEFAULT(m_bRecording, 1606, 1606)
OFFSET_DEFAULT(m_nDemoNumber, 1608, 1608)

// CDemoPlayer
OFFSET_DEFAULT(GetPlaybackTick, 3, 4)
OFFSET_DEFAULT(StartPlayback, 5, 6)
OFFSET_DEFAULT(StopPlayback, 16, 17)
OFFSET_DEFAULT(IsPlayingBack, 6, 7)
OFFSET_DEFAULT(IsPlaybackPaused, 7, 8)
OFFSET_DEFAULT(IsSkipping, 9, 10)
OFFSET_DEFAULT(SkipToTick, 13, 14)
OFFSET_DEFAULT(m_szFileName, 4, 4)

// CClientState
OFFSET_DEFAULT(ProcessTick, 1, 12)
OFFSET_DEFAULT(Disconnect, 16, 37)
OFFSET_DEFAULT(viewangles, 19040, 19012)

// CBaseEntity
OFFSET_DEFAULT(IsPlayer, 85, 86)
OFFSET_DEFAULT(AcceptInput, 40, 41)
OFFSET_DEFAULT(S_GetDataDescMap, 11, 12)
OFFSET_DEFAULT(C_GetDataDescMap, 15, 17)
OFFSET_DEFAULT(GetServerClass, 9, 10)
OFFSET_DEFAULT(GetClientClass, 18, 19)
OFFSET_DEFAULT(GetPredDescMap, 17, 20)

// CBasePlayer
OFFSET_DEFAULT(m_pSurfaceData, 3868, 4116)
OFFSET_DEFAULT(m_pShadowStand, 3160, 3184) // "player_stand" -> CBasePlayer::InitVCollision -> CBasePlayer::SetupVPhysicsShadow -> usage of pStandHullName -> vtable offset write
OFFSET_DEFAULT(m_pShadowCrouch, 3164, 3188) // ^ "player_crouch" pCrouchHullName
OFFSET_DEFAULT(S_m_surfaceFriction, 4096, 4120)
OFFSET_DEFAULT(C_m_surfaceFriction, 5548, 5520)

// CPortal_Player
OFFSET_DEFAULT(GetPaintPower, 2, 513)
OFFSET_DEFAULT(UseSpeedPower, 509, 519)
OFFSET_DEFAULT(PlayerRunCommand, 453, 454)

// CProp_Portal
OFFSET_DEFAULT(m_fStaticAmount, 13584, 13552)
OFFSET_DEFAULT(m_fSecondaryStaticAmount, 13588, 13556)
OFFSET_DEFAULT(m_fOpenAmount, 13592, 13560)

// IEngineVGuiInternal
OFFSET_DEFAULT(IsGameUIVisible, 2, 2)
OFFSET_DEFAULT(Paint, 14, 15)

// IEngineTrace
OFFSET_DEFAULT(TraceRay, 5, 5)

// CEngineTool
OFFSET_DEFAULT(GetCurrentMap, 25, 26)
OFFSET_DEFAULT(HostFrameTime, 39, 40)
OFFSET_DEFAULT(ClientTime, 47, 48)
OFFSET_DEFAULT(PrecacheModel, 61, 61)
OFFSET_DEFAULT(ClientTick, 49, 50)
OFFSET_DEFAULT(ServerTick, 45, 46)
OFFSET_DEFAULT(HostTick, 41, 42)

// CSchemeManager
OFFSET_DEFAULT(GetIScheme, 8, 9)

// CScheme
OFFSET_DEFAULT(GetFont, 3, 4)

// IClientEntityList
OFFSET_DEFAULT(GetClientEntity, 3, 3)

// CServerGameDLL
OFFSET_DEFAULT(GameFrame, 4, 4)
OFFSET_DEFAULT(ApplyGameSettings, 38, 38)
OFFSET_DEFAULT(Think, 31, 31)
OFFSET_DEFAULT(GetAllServerClasses, 10, 10)
OFFSET_DEFAULT(IsRestoring, 24, 24)
OFFSET_DEFAULT(ShouldDraw, 11, 12)

// CGlobalEntityList
OFFSET_DEFAULT(OnRemoveEntity, 1, 1)

// CHud
OFFSET_DEFAULT(GetName, 10, 11)

// CGameEventManager
OFFSET_DEFAULT(AddListener, 3, 4)
OFFSET_DEFAULT(RemoveListener, 5, 6)
OFFSET_DEFAULT(FireEventClientSide, 8, 9)
OFFSET_DEFAULT(FireEventIntern, 12, 16)
OFFSET_DEFAULT(ConPrintEvent, 303, 481)

// CEngine
OFFSET_DEFAULT(Frame, 5, 6)

// CEngineAPI
OFFSET_DEFAULT(IsRunningSimulation, 12, 12)
OFFSET_DEFAULT(Init, 3, 3)

// CIVDebugOverlay
OFFSET_DEFAULT(ScreenPosition, 12, 11)

// CCommandBuffer
OFFSET_DEFAULT(m_bWaitEnabled, 8265, 8265)

// CServerTools
OFFSET_DEFAULT(GetIServerEntity, 1, 2)
OFFSET_DEFAULT(CreateEntityByName, 14, 15)
OFFSET_DEFAULT(DispatchSpawn, 15, 16)
OFFSET_DEFAULT(SetKeyValueChar, 13, 12)
OFFSET_DEFAULT(SetKeyValueFloat, 12, 13)
OFFSET_DEFAULT(SetKeyValueVector, 11, 14)

// CVEngineServer
OFFSET_DEFAULT(ClientCommand, 39, 39)
OFFSET_DEFAULT(ClientCommandKeyValues, 135, 135)
OFFSET_DEFAULT(IsServerPaused, 81, 81)
OFFSET_DEFAULT(ServerPause, 121, 121)
OFFSET_DEFAULT(ChatPrintf, 22, 25)
OFFSET_DEFAULT(MsgFunc_SayText2, 28, 35)
OFFSET_DEFAULT(MsgFunc_SayTextReloaded, 26, 32)
OFFSET_DEFAULT(GetTextColorForClient, 33, 41)

// CSteam3Client
OFFSET_DEFAULT(OnGameOverlayActivated, 148, 156)

// surfacedata_t
OFFSET_DEFAULT(jumpFactor, 68, 72)

// IPhysicsObject
OFFSET_DEFAULT(IsAsleep, 2, 3)
OFFSET_DEFAULT(IsCollisionEnabled, 6, 7)
OFFSET_DEFAULT(IsGravityEnabled, 7, 8)
OFFSET_DEFAULT(IsDragEnabled, 8, 9)
OFFSET_DEFAULT(IsMotionEnabled, 9, 10)
OFFSET_DEFAULT(GetPosition, 48, 49)
OFFSET_DEFAULT(GetVelocity, 52, 53)
OFFSET_DEFAULT(SetPosition, 46, 47)
OFFSET_DEFAULT(SetVelocity, 50, 51)
OFFSET_DEFAULT(EnableGravity, 13, 14)
OFFSET_DEFAULT(GetCollide, 74, 75)
OFFSET_DEFAULT(Wake, 24, 25)
OFFSET_DEFAULT(Sleep, 25, 26)

// IVideoMode
OFFSET_DEFAULT(GetModeWidth, 14, 15)
OFFSET_DEFAULT(GetModeHeight, 15, 16)
OFFSET_DEFAULT(ReadScreenPixels, 28, 29)

// IFileSystem
OFFSET_DEFAULT(GetSearchPath, 16, 16)

// CGameRules
OFFSET_DEFAULT(IsMultiplayer, 33, 34)

// IClientRenderable
OFFSET_DEFAULT(GetModel, 8, 8)

// Others
OFFSET_DEFAULT(tickcount, 95, 64)
OFFSET_DEFAULT(interval_per_tick, 65, 58)
OFFSET_DEFAULT(GetClientStateFunction, 4, 9)
OFFSET_EMPTY(cl)
OFFSET_DEFAULT(demoplayer, 74, 80)
OFFSET_DEFAULT(demorecorder, 87, 93)
OFFSET_DEFAULT(m_szLevelName, 36, 38)
OFFSET_DEFAULT(AirMove_Offset1, 7, 11)
OFFSET_DEFAULT(AirMove_Offset2, 5, 9)
OFFSET_DEFAULT(UTIL_PlayerByIndex, 39, 53)
OFFSET_DEFAULT(gpGlobals, 14, 9)
OFFSET_DEFAULT(g_Input, 2, 1)
OFFSET_EMPTY(in_jump)
OFFSET_DEFAULT(GetClientMode, 4, 9)
OFFSET_EMPTY(State_Shutdown)
OFFSET_DEFAULT(Cbuf_AddText, 46, 35)
OFFSET_DEFAULT(s_CommandBuffer, 82, 52)
OFFSET_DEFAULT(CCommandBufferSize, 9556, 9556)
OFFSET_EMPTY(AddText)
OFFSET_DEFAULT(StartDrawing, 127, 1341)
OFFSET_DEFAULT(FinishDrawing, 603, 355)
OFFSET_DEFAULT(GetHud, 125, 130)
OFFSET_DEFAULT(FindElement, 135, 143)
OFFSET_DEFAULT(Key_SetBinding, 135, 73)
OFFSET_DEFAULT(eng, 2, 4)
OFFSET_DEFAULT(HostState_OnClientConnected, 684, 1275)
OFFSET_DEFAULT(hoststate, 1, 28)
OFFSET_DEFAULT(m_bLoadGame, 448, 440)
OFFSET_DEFAULT(NUM_ENT_ENTRIES, 8192, 8192)
OFFSET_DEFAULT(ENT_ENTRY_MASK, 65535, 65535)
OFFSET_DEFAULT(INVALID_EHANDLE_INDEX, 0xFFFFFFFF, 0xFFFFFFFF)
OFFSET_DEFAULT(NUM_SERIAL_NUM_SHIFT_BITS, 16, 16)
OFFSET_DEFAULT(m_EntPtrArray, 61, 45)
OFFSET_DEFAULT(g_pClientMode, 19, 20)
OFFSET_DEFAULT(m_pCommands, 244, 244)
OFFSET_DEFAULT(CUserCmdSize, 96, 96)
OFFSET_DEFAULT(MULTIPLAYER_BACKUP, 150, 150)
OFFSET_DEFAULT(PerUserInput_tSize, 376, 352)
OFFSET_DEFAULT(GetLocalClient, 128, 83)
OFFSET_DEFAULT(MAX_SPLITSCREEN_PLAYERS, 2, 2)
OFFSET_DEFAULT(net_time, 19, 29)
OFFSET_DEFAULT(VideoMode_Create, 88, 103)
OFFSET_DEFAULT(videomode, 35, 178)
OFFSET_EMPTY(VID_ProcessMovieFrame_1)
OFFSET_EMPTY(VID_ProcessMovieFrame_2)
OFFSET_DEFAULT(snd_linear_count, 63, 69)
OFFSET_DEFAULT(snd_p, 98, 116)
OFFSET_DEFAULT(snd_vol, 108, 122)
OFFSET_DEFAULT(StartTouch, 102, 103)
OFFSET_DEFAULT(UncacheUnusedMaterials, 77, 77)
OFFSET_DEFAULT(CreateMaterial, 81, 81)
OFFSET_DEFAULT(FindMaterial, 82, 82)
OFFSET_DEFAULT(CreateProceduralTexture, 91, 91)
OFFSET_DEFAULT(RemoveMaterial, 156, 157)
OFFSET_DEFAULT(DecrementReferenceCount, 13, 13)
OFFSET_DEFAULT(g_pTextureManager, 8, 4)
OFFSET_DEFAULT(RemoveTexture, 30, 30)
OFFSET_DEFAULT(GetRenderContext, 113, 113)
OFFSET_DEFAULT(CreateDebugMesh, 41, 42)
OFFSET_DEFAULT(DestroyDebugMesh, 42, 43)
OFFSET_DEFAULT(ImageData, 39, 38)
OFFSET_DEFAULT(ImageFormat, 20, 21)
OFFSET_DEFAULT(CBaseEntity_Create, 130, 145)
OFFSET_DEFAULT(m_pLeaderboard, 2308, 3328)
OFFSET_DEFAULT(m_pStatList, 2252, 3272)
OFFSET_DEFAULT(m_nStatHeight, 2100, 3120)

OFFSET_DEFAULT(g_movieInfo, 5, 2)
OFFSET_DEFAULT(SND_IsRecording, 0, 21)

OFFSET_DEFAULT(gamerules, 2, 1)
OFFSET_DEFAULT(OpenRadialMenuCommand, -1, 9)

OFFSET_DEFAULT(nNumSPChapters, 11, 12)
OFFSET_DEFAULT(g_ChapterContextNames, 16, 22)
OFFSET_DEFAULT(nNumMPChapters, 23, 81)
OFFSET_DEFAULT(g_ChapterMPContextNames, 28, 91)

OFFSET_DEFAULT(GetLeaderboard, 290, 973)
OFFSET_DEFAULT(IsQuerying, 366, 666)
OFFSET_DEFAULT(SetPanelStats, 413, 1056)
OFFSET_DEFAULT(StartSearching, 172, 262)
OFFSET_DEFAULT(AddAvatarPanelItem, 1102, 1107)
OFFSET_DEFAULT(PurgeAndDeleteElements, 37, 120)

OFFSET_DEFAULT(host_frametime, 92, 81)

OFFSET_DEFAULT(say_callback_insn, 52, 55)
OFFSET_DEFAULT(Host_Say_insn, 0x335, 0x36C)
OFFSET_DEFAULT(portalsThruPortals, 391, 388)

// clang-format off

// Pathmatch
SIGSCAN_DEFAULT(PathMatch, "", "55 57 56 53 83 EC 0C 8B 6C 24 28 8B 5C 24 2C 0F B6 05 ? ? ? ? 84 C0 0F 84 ? ? ? ?")


// Renderer
SIGSCAN_DEFAULT(SND_RecordBuffer, "55 8B EC 80 3D ? ? ? ? 00 53 56 57 0F 84 15 01 00 00 E8 ? ? ? ? 84 C0 0F 85 08 01 00 00 A1 ? ? ? ? 3B 05",
                                  "80 3D ? ? ? ? 00 75 07 C3 ? ? ? ? ? ? 55 89 E5 57 56 53 83 EC 1C E8 ? ? ? ? 84 C0 0F 85 ? ? ? ?") // "DS_STEREO" -> CAudioDirectSound::TransferSamples -> S_TransferStereo16 -> SND_RecordBuffer


// Client
SIGSCAN_DEFAULT(MatrixBuildRotationAboutAxis, "55 8B EC 51 F3 0F 10 45 ? 0F 5A C0 F2 0F 59 05 ? ? ? ? 66 0F 5A C0 F3 0F 11 45 ? E8 ? ? ? ? F3 0F 11 45 ? F3 0F 10 45 ? E8 ? ? ? ? 8B 45 ? F3 0F 10 08",
                                              "56 66 0F EF C0 53 83 EC 14 8B 5C 24 ? 8D 44 24")
SIGSCAN_DEFAULT(DrawTranslucentRenderables, "55 8B EC 81 EC 80 00 00 00 53 56 8B F1 8B 0D ? ? ? ? 8B 01 8B 90 C4 01 00 00 57 89 75 F0 FF D2 8B F8",
                                            "55 89 E5 57 56 53 81 EC B8 00 00 00 8B 45 10 8B 5D 0C 89 85 60 FF FF FF 88 45 A7 A1 ? ? ? ?")
SIGSCAN_DEFAULT(DrawOpaqueRenderables, "55 8B EC 83 EC 54 83 7D 0C 00 A1 ? ? ? ? 53 56 0F 9F 45 EC 83 78 30 00 57 8B F1 0F 84 BA 03 00 00",
                                       "55 89 E5 57 56 53 83 EC 7C A1 ? ? ? ? 8B 5D 08 89 45 90 85 C0 0F 85 34 04 00 00 A1 ? ? ? ? 8B 40 30 85 C0")
SIGSCAN_DEFAULT(MsgPreSkipToNextLevel, "57 8B F9 E8 ? ? ? ? 8B C8 E8 ? ? ? ? 0B C2",
                                       "53 83 EC 08 E8 ? ? ? ? 83 EC 0C 50 E8 ? ? ? ? 83 C4 10 09 C2")
SIGSCAN_DEFAULT(CalcViewModelLag, "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 1C 56 6A 00 6A 00 8D 45 F4 8B F1 8B 4B 0C 50 51 E8 ? ? ? ?",
                                  "56 53 83 EC 24 8B 74 24 30 8B 5C 24 34 6A 00 6A 00 8D 44 24 1C 50 FF 74 24 44 E8 ? ? ? ? A1 ? ? ? ? 83 C4 10 66 0F EF C9")
SIGSCAN_DEFAULT(AddShadowToReceiver, "55 8B EC 51 53 56 57 0F B7 7D 08",
                                     "55 89 E5 57 56 53 83 EC 44 8B 45 0C 8B 5D 08 8B 55 14 8B 75 10")
SIGSCAN_DEFAULT(UTIL_Portal_Color, "55 8B EC 56 8B 75 ? 85 F6 0F 84 ? ? ? ? 0F 8E",
                                    "56 53 83 EC 04 8B 44 24 ? 8B 74 24 ? 85 C0 74 ? 8D 58")
SIGSCAN_DEFAULT(UTIL_Portal_Color_Particles, "55 8B EC 51 8B 0D ? ? ? ? 8B 01 8B 90 ? ? ? ? FF D2 84 C0",
                                    "53 83 EC 14 A1 ? ? ? ? 8B 5C 24 ? 8B 10 50 FF 92 ? ? ? ? 83 C4 10 84 C0 75")
SIGSCAN_DEFAULT(GetNumChapters, "55 8B EC 80 7D 08 00 57 74 0C",
                                "55 89 E5 56 80 7D")
SIGSCAN_DEFAULT(CPortalLeaderboardPanel_OnThink, "55 8B EC A1 ? ? ? ? 81 EC ? ? ? ? 53 56 32 DB",
                                                 "55 89 E5 57 56 53 81 EC ? ? ? ? 65 A1 ? ? ? ? 89 45 E4 31 C0 A1 ? ? ? ? 8B 5D 08 8B 70 30")
SIGSCAN_DEFAULT(OnEvent, "55 8B EC 57 8B F9 8B 4D 08 E8",
                         "55 89 E5 57 56 53 83 EC 1C 8B 45 0C 8B 7D 08 89 04 24 E8 ? ? ? ? C7 04 24")
SIGSCAN_DEFAULT(OnCommand, "55 8B EC 56 57 8B 7D 08 57 68 ? ? ? ? 8B F1 E8 ? ? ? ? 83 C4 08 85 C0 0F 84",
                           "55 89 E5 57 56 53 83 EC 2C 8B 75 0C C7 04 24 ? ? ? ? 8B 5D 08 89 74 24 04 E8 ? ? ? ? 85 C0 75 3D")

SIGSCAN_DEFAULT(GetChapterProgress, "56 8B 35 ? ? ? ? 57 8B F9 FF D6 8B 10 8B C8",
                                    "55 89 E5 57 56 53 83 EC 0C E8 ? ? ? ? 83 EC 08 8B 10")


// Engine
SIGSCAN_DEFAULT(ParseSmoothingInfoSig, "55 8B EC 0F 57 C0 81 EC ? ? ? ? B9 ? ? ? ? 8D 85 ? ? ? ? EB", "");
OFFSET_DEFAULT(ParseSmoothingInfoOff, 178, -1)
SIGSCAN_DEFAULT(Host_AccumulateTime, "55 8B EC 51 F3 0F 10 05 ? ? ? ? F3 0F 58 45 08 8B 0D ? ? ? ? F3 0F 11 05 ? ? ? ? 8B 01 8B 50 20 53 B3 01 FF D2",
                                     "83 EC 1C 8B 15 ? ? ? ? F3 0F 10 05 ? ? ? ? F3 0F 58 44 24 20 F3 0F 11 05 ? ? ? ? 8B 02 8B 40 24 3D ? ? ? ? 0F 85 41 03 00 00")
SIGSCAN_DEFAULT(_Host_RunFrame_Render, "A1 ? ? ? ? 85 C0 75 1B 8B 0D ? ? ? ? 8B 01 8B 50 40 68 ? ? ? ? FF D2 A3 ? ? ? ? 85 C0 74 0D 6A 02 6A F6 50 E8 ? ? ? ? 83 C4 0C",
                                       "55 89 E5 57 56 53 83 EC 1C 8B 1D ? ? ? ? 85 DB 0F 85 69 02 00 00 E8 64 FF FF FF A1 ? ? ? ? 80 3D C5 ? ? ? ? 8B 78 30 74 12 83 EC 08 6A 00")
SIGSCAN_DEFAULT(readCustomDataInjectSig, "8D 45 E8 50 8D 4D BC 51 8D 4F 04 E8 ? ? ? ? 8B 4D BC 83 F9 FF", // "Unable to decode custom demo data, callback \"%s\" not found.\n" -> memory reference -> first function call
                                         "8D 85 C4 FE FF FF 83 EC 04 8D B5 E8 FE FF FF 56 50 FF B5 94 FE FF FF E8")
OFFSET_DEFAULT(readCustomDataInjectOff, 12, 24)
SIGSCAN_DEFAULT(readConsoleCommandInjectSig, "8B 45 F4 50 68 FE 04 00 00 68 ? ? ? ? 8D 4D 90 E8 ? ? ? ? 8D 4F 04 E8",
                                             "FF B5 AC FE FF FF 8D B5 E8 FE FF FF 68 FE 04 00 00 68 ? ? ? ? 56 E8 ? ? ? ? 58 FF B5 94 FE FF FF E8") // "%d dem_consolecmd [%s]\n" -> memory reference -> second function call
OFFSET_DEFAULT(readConsoleCommandInjectOff, 26, 36)
SIGSCAN_DEFAULT(Cmd_ExecuteCommand, "55 8B EC 57 8B 7D ? 8B 07 85 C0",
                                    "55 89 E5 57 56 53 83 EC 2C 8B 75 ? 8B 3E") // "WARNING: INVALID EXECUTION MARKER.\n"
SIGSCAN_DEFAULT(InsertCommand, "55 8B EC 56 57 8B 7D ? 8B F1 81 FF FF 01 00 00",
                               "55 57 56 53 83 EC 1C 8B 6C 24 ? 8B 5C 24 ? 8B 74 24 ? 81 FD FE 01 00 00") // "WARNING: Command too long... ignoring!\n%s\n"
SIGSCAN_DEFAULT(Convar_PrintDescription, "25 2D 38 30 73 20 2D 20 25 2E 38 30 73 0A 00",
                                         "25 2D 38 30 73 20 2D 20 25 2E 38 30 73 0A 00") // "%-80s - %.80s\n"


// EngineDemoPlayer
SIGSCAN_DEFAULT(InterpolateDemoCommand, "55 8B EC 83 EC 10 56 8B F1 8B 4D 10 57 8B BE B4 05 00 00 83 C1 04 89 75 F4 89 7D F0 E8 ? ? ? ? 8B 4D 14 83 C1 04",
                                        "55 57 56 53 83 EC 10 8B 44 24 24 8B 5C 24 2C 8B 88 B0 05 00 00 8B 44 24 30 8D 70 04 8D 90 9C 00 00 00 89 F0 F3 0F 10 40 04")


// Matchmaking
SIGSCAN_DEFAULT(UpdateLeaderboardData, "55 8B EC 83 EC 08 53 8B D9 8B 03 8B 50 08",
                                       "55 89 E5 57 56 53 83 EC 2C 8B 45 08 8B 5D 0C")


// MaterialSystem
SIGSCAN_DEFAULT(KeyValues_SetString, "55 8B EC 8B 45 08 6A 01 50 E8 ? ? ? ? 85 C0 74 0B",
                                     "53 83 EC ? 8B 5C 24 ? 6A ? FF 74 24 ? FF 74 24 ? E8 ? ? ? ? 83 C4 ? 85 C0 74 ? 89 5C 24")


// Server
SIGSCAN_DEFAULT(GlobalEntity_GetIndex, "55 8B EC 51 8B 45 08 50 8D 4D FC 51 B9 ? ? ? ? E8 ? ? ? ? 66 8B 55 FC B8 FF FF 00 00",
                                       "53 83 EC 18 8D 44 24 0E 83 EC 04 FF 74 24 24 68 ? ? ? ? 50 E8 ? ? ? ? 0F B7 4C 24 1A 83 C4 0C 66 83 F9 FF 74 35 8B 15 ? ? ? ? 89 D0")
SIGSCAN_DEFAULT(GlobalEntity_SetFlags, "55 8B EC 80 3D ? ? ? ? 00 75 1F 8B 45 08 85 C0 78 18 3B 05 ? ? ? ? 7D 10 8B 4D 0C 8B 15 ? ? ? ? 8D 04 40 89 4C 82 08",
                                       "80 3D ? ? ? ? 01 8B 44 24 04 74 1F 85 C0 78 1B 3B 05 ? ? ? ? 7D 13 8B 15 ? ? ? ? 8D 04 40")
SIGSCAN_DEFAULT(Host_Say, "55 8B EC 81 EC 30 02 00 00 56",
                          "55 89 E5 57 56 53 81 EC 4C 02 00 00 8B 45")
SIGSCAN_DEFAULT(TraceFirePortal, "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC 38 07 00 00 56 57 8B F1",
                                 "55 89 E5 57 56 8D B5 F4 F8 FF FF 53 81 EC 30 07 00 00 8B 45 14 6A 00 8B 5D 0C FF 75 08 56 89 85 D0 F8 FF FF")
SIGSCAN_DEFAULT(FindPortal, "55 8B EC 0F B6 45 08 8D 0C 80 03 C9 53 8B 9C 09 ? ? ? ? 03 C9 56 57 85 DB 74 3C 8B B9 ? ? ? ? 33 C0 33 F6 EB 08",
                            "55 57 56 53 83 EC 2C 8B 44 24 40 8B 74 24 48 8B 7C 24 44 89 44 24 14 0F B6 C0 8D 04 80 89 74 24 0C C1 E0 02")
SIGSCAN_DEFAULT(ViewPunch, "55 8B EC A1 ? ? ? ? 83 EC 0C 83 78 30 00 56 8B F1 0F 85 ? ? ? ? 8B 16 8B 82 00 05 00 00 FF D0 84 C0 0F 85 ? ? ? ? 8B 45 08 F3 0F 10 1D ? ? ? ? F3 0F 10 00",
                           "55 57 56 53 83 EC 1C A1 ? ? ? ? 8B 5C 24 30 8B 74 24 34 8B 40 30 85 C0 75 38 8B 03 8B 80 04 05 00 00 3D ? ? ? ? 75 36 8B 83 B8 0B 00 00 8B 0D ? ? ? ?")
SIGSCAN_DEFAULT(UTIL_FindClosestPassableSpace, "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC 98 02 00 00 8B 43 0C 8B 48 08 F3 0F 10 48 04 F3 0F 10 00 F3 0F 10 3D ? ? ? ?",
                                               "55 BA 00 01 00 00 66 0F EF ED 66 0F EF C0 57 56 53 81 EC CC 02 00 00 8B 0D ? ? ? ? 8B 84 24 E4 02 00 00 66 89 94 24 54 01 00 00 8B 3D ? ? ? ?")
SIGSCAN_DEFAULT(FindClosestPassableSpace, "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC A1 ? ? ? ? 81 EC 88 02 00 00 83 78 30 00 56 57 0F 84 ? ? ? ? 8B 73 08 8B 8E DC 00 00 00",
                                          "A1 ? ? ? ? 57 56 53 8B 5C 24 10 8B 74 24 14 8B 50 30 8B 4C 24 18 85 D2 74 29 8B 83 E4 00 00 00 8B 3D ? ? ? ? 83 F8 FF 74 24 0F B7 D0 C1 E8 10")
SIGSCAN_DEFAULT(UTIL_GetCommandClientIndex, "A1 ? ? ? ? 40 C3",
                                            "A1 ? ? ? ? 83 C0 01 C3")
SIGSCAN_DEFAULT(CheckStuck_FloatTime, "FF ? ? ? ? ? D9 5D F8 8B 56 04 8B 42 1C 8B ? ? ? ? ? 3B C3 75 04 33 C9 EB 08 8B C8 2B 4A 58 C1 F9 04 F3 0F 10 84 CE 70",
                                      "E8 ? ? ? ? 8B 43 04 66 0F EF C0 DD 5C 24 08 F2 0F 5A 44 24 08 8B 40 24 85 C0 0F 84 CC 01 00 00 8B 15 ? ? ? ? 2B 42 58")
SIGSCAN_DEFAULT(IsInPVS, "55 8B EC 51 53 8B 5D 08 56 57 33 FF 89 4D FC 66 39 79 1A 75 57 3B BB 10 20 00 00 0F 8D C0 00 00 00 8D B3 14 20 00 00",
                         "55 57 56 53 31 DB 83 EC 0C 8B 74 24 20 8B 7C 24 24 66 83 7E 1A 00 8B 87 10 20 00 00 89 C2 0F 85 BC 00 00 00 85 C0 7F 75 8D B4 26")
SIGSCAN_DEFAULT(CreateViewModel, "E8 ? ? ? ? 5F 5D C2 04 00 53",
                                 "E8 ? ? ? ? E9 ? ? ? ? 8D B4 26 00 00 00 00 8D 76 00 8B 3D")
SIGSCAN_DEFAULT(aircontrol_fling_speedSig, "0F 2F 25 ? ? ? ? F3 0F 11 45",
                                           "0F 2F 05 ? ? ? ? 0F 86 ? ? ? ? 0F 2F D1") // "%s: Make in time? %s velocity %f wish %f\n" -> CPortalGameMovement::AirPortalFunnel -> xref CPortalGameMovement::PortalFunnel -> xref(has 90000) CPortalGameMovement::AirMove, [U]COMISS XMM dword ptr
OFFSET_DEFAULT(aircontrol_fling_speedOff, 3, 3)


// Steam API
SIGSCAN_DEFAULT(interfaceMgrSig, "89 0D ? ? ? ? 85 C9 0F", "")
OFFSET_DEFAULT(interfaceMgrOff, 2, -1)

// clang-format on

#ifndef _WIN32
#pragma GCC diagnostic pop
#endif
