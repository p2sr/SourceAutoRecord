#pragma once

#include "Game.hpp"

namespace Offsets
{
	// CCvar
	int FindVar;
	int m_pConCommandList;

	// CEngineClient
	int ClientCmd;
	int GetLocalPlayer;
	int GetGameDirectory;
	int GetViewAngles;
	int SetViewAngles;

	// CHLClient
	int HudUpdate;

	// ConVar
	int InternalSetValue;
	int InternalSetFloatValue;
	int InternalSetIntValue;

	// IMatSystemSurface
	int GetFontTall;
	int DrawColoredText;

	// CInputSystem
	int StringToButtonCode;

	// CGameMovement
	int CheckJumpButton;
	int mv;
	int m_nOldButtons;

	// CDemoRecorder
	int GetRecordingTick;
	int SetSignonState;
	int StopRecording;
	int m_szDemoBaseName;
	int m_bRecording;
	int m_nDemoNumber;

	// CDemoPlayer
	int GetPlaybackTick;
	int StartPlayback;
	int IsPlayingBack;
	int m_szFileName;

	// CClientState
	int ProcessTick;
	int Disconnect;

	// C_BasePlayer
	int GetAbsOrigin;
	int GetAbsAngles;
	int GetLocalVelocity;

	// IEngineVGuiInternal
	int Paint;

	// CEngineTool
	int GetCurrentMap;

	// CSchemeManager
	int GetIScheme;

	// CScheme
	int GetFont;

	// IClientEntityList
	int GetClientEntity;

	// Others
	int MainViewOrigin;
	int MainViewAngles;
	int tickcount;
	int interval_per_tick;
	int GetClientState;
	int demoplayer;
	int demorecorder;
	int m_szLevelName;

	void Init()
	{
		if (Game::Version == Game::Portal2) {
			// engine.so
			InternalSetValue = 12; // tier1/convar.cpp
			InternalSetFloatValue = 13; // tier1/convar.cpp
			InternalSetIntValue = 14; // tier1/convar.cpp
			FindVar = 15; // CCvarUtilities::IsValidToggleCommand
			m_pConCommandList = 48; // vstdlib/cvar.cpp
			ClientCmd = 7; // CEngineClient
			GetLocalPlayer = 12; // CEngineClient
			GetGameDirectory = 35; // CEngineClient
			GetViewAngles = 18; // CEngineClient
			SetViewAngles = 19; // CEngineClient
			StringToButtonCode = 31; // ReadCheatCommandsFromFile
			GetRecordingTick = 1; // CDemoRecorder
			SetSignonState = 3; // CDemoRecorder
			StopRecording = 7; // CDemoRecorder
			GetPlaybackTick = 3; // CDemoPlayer
			StartPlayback = 6; // CDemoPlayer
			IsPlayingBack = 7; // CDemoPlayer
			m_szFileName = 4; // CDemoPlayer::SkipToTick
			m_szDemoBaseName = 1344; // CDemoRecorder::StartupDemoFile
			m_nDemoNumber = 1608; // CDemoRecorder::StartupDemoFile
			m_bRecording = 1606; // CDemoRecorder::SetSignonState
			Paint = 15; // IEngineVGuiInternal
			ProcessTick = 12; // CClientState
			tickcount = 73; // CClientState::ProcessTick
			interval_per_tick = 81; // CClientState::ProcessTick
			Disconnect = 37; //  CClientState
			GetClientState = 11; // CEngineClient::ClientCmd
			GetCurrentMap = 26; // CEngineTool
			demoplayer = 93; // CClientState::Disconnect
			demorecorder = 106; // CClientState::Disconnect
			m_szLevelName = 72; // CEngineTool::GetCurrentMap
			GetIScheme = 9; // CSchemeManager
			GetFont = 4; // CScheme
			// server.so
			CheckJumpButton = 37; // CPortalGameMovement
			mv = 2; // CPortalGameMovement::CheckJumpButton
			m_nOldButtons = 40; // CPortalGameMovement::CheckJumpButton
			// client.so
			HudUpdate = 11; // CHLClient
			GetFontTall = 72; // CFPSPanel::ComputeSize
			DrawColoredText = 160; // CFPSPanel::Paint
			GetAbsOrigin = 136; // C_BasePlayer::GetAbsOrigin
			GetAbsAngles = 172; // C_BasePlayer::GetAbsAngles
			GetLocalVelocity = 244; // CFPSPanel::Paint
			MainViewOrigin = 39; // GetPos view.cpp
			MainViewAngles = 65; // GetPos view.cpp
			GetClientEntity = 3; // IClientEntityList
		}
	}
}