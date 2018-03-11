#pragma once

#include "Game.hpp"

namespace Patterns
{
	namespace Portal2
	{
		void Load();
	}
	
	std::vector<Pattern> Items;

	void Create(const char* moduleName, const char* patternName)
	{
		Items.push_back(Pattern
		{
			moduleName,
			patternName
		});
	}
	Pattern* Get(const char* patternName)
	{
		for (auto &pattern : Items) {
			if (pattern.Name == patternName) {
				return &pattern;
			}
		}
		return nullptr;
	}
	void Add(const char* patternName, const char* version, const char* sigName, const char* sigBytes, const int offset = 0)
	{
		Get(patternName)->SetSignature
		(
			version,
			sigName,
			sigBytes,
			offset
		);
	}
	void Inherit(const char* patternName, const char* version, const char* sigName)
	{
		Get(patternName)->SetSignature
		(
			version,
			sigName
		);
	}
	void Init()
	{
		Create("server.so", "CheckJumpButton");
		Create("server.so", "PlayerUse");
		Create("server.so", "AirMove");
		Create("server.so", "PlayerRunCommand");

		Create("client.so", "SetSize");
		Create("client.so", "GetLocalPlayer");
		Create("client.so", "GetPos");
		Create("client.so", "g_pMatSystemSurface");
		Create("client.so", "Paint");
		Create("client.so", "ShouldDraw");
		Create("client.so", "FindElement");

		Create("engine.so", "CvarPtr");
		Create("engine.so", "ConVar_Ctor3");
		Create("engine.so", "ConCommand_Ctor1");
		Create("engine.so", "ConCommand_Ctor2");
		Create("engine.so", "engineClient");
		Create("engine.so", "GetGameDir");
		Create("engine.so", "curtime");
		Create("engine.so", "m_bLoadgame");
		Create("engine.so", "m_szMapname");
		Create("engine.so", "m_currentState");
		Create("engine.so", "demorecorder");
		Create("engine.so", "g_pInputSystem");
		Create("engine.so", "Key_SetBinding");
		Create("engine.so", "demoplayer");
		Create("engine.so", "SetSignonState");
		Create("engine.so", "CloseDemoFile");
		Create("engine.so", "StopRecording");
		Create("engine.so", "StartupDemoFile");
		Create("engine.so", "ConCommandStop");
		Create("engine.so", "Disconnect");
		Create("engine.so", "PlayDemo");
		Create("engine.so", "StartPlayback");
		Create("engine.so", "HostStateFrame");
		Create("engine.so", "PrintDescription");
		Create("engine.so", "ReadPacket");

		if (Game::Version == Game::Portal2){
			Portal2::Load();
		}
	}

	namespace Portal2
	{
		void Load()
		{
			// server.so


			// \x55\x89\xE5\x83\xEC\x00\x89\x5D\x00\x8B\x5D\x00\x89\x75\x00\x31\xF6\x8B\x43\x00\x80\xB8\x00\x00\x00\x00\x00 xxxxx?xx?xx?xx?xxxx?xx?????
			Add("CheckJumpButton", "Portal 2 Build 7054",
				"CPortalGameMovement::CheckJumpButton",
				"55 89 E5 83 EC ? 89 5D ? 8B 5D ? 89 75 ? 31 F6 8B 43 ? 80 B8 ? ? ? ? ? ");

			// \x55\x31\xC0\x89\xE5\x57\x56\x53\x83\xEC\x00\x8B\x5D\x00\x8B\x0D\x00\x00\x00\x00 xxxxxxxxxx?xx?xx????
			Add("PlayerUse", "Portal 2 Build 7054",
				"CPortal_Player::PlayerUse",
				"55 31 C0 89 E5 57 56 53 83 EC ? 8B 5D ? 8B 0D ? ? ? ? ");

			// \x55\x89\xE5\x57\x56\x8D\x45\x00\x53\x8D\x75\x00\x8D\x7D\x00\x83\xEC\x00 xxxxxxx?xxx?xx?xx?
			Add("AirMove", "Portal 2 Build 7054",
				"CPortalGameMovement::AirMove",
				"55 89 E5 57 56 8D 45 ? 53 8D 75 ? 8D 7D ? 83 EC ? ",
				540);

			// \x55\x89\xE5\x56\x53\x83\xEC\x00\x8B\x5D\x00\x8B\x75\x00\x8B\x83\x00\x00\x00\x00\xC6\x83\x00\x00\x00\x00\x00 xxxxxxx?xx?xx?xx????xx?????
			Add("PlayerRunCommand", "Portal 2 Build 7054",
				"CBasePlayer::PlayerRunCommand",
				"55 89 E5 56 53 83 EC ? 8B 5D ? 8B 75 ? 8B 83 ? ? ? ? C6 83 ? ? ? ? ? ",
				67);


			// client.so


			// \x55\x89\xE5\x83\xEC\x00\x89\x5D\x00\x8B\x1D\x00\x00\x00\x00\x89\x75\x00\x8B\x45\x00\x8B\x13\x8B\x72\x00\x8B\x10\x89\x04\x24\xFF\x12\x8B\x55\x00\x89\x1C\x24\x89\x44\x24\x00\x89\x54\x24\x00\x8B\x55\x00\x89\x54\x24\x00\xFF\xD6\x8B\x5D\x00\x8B\x75\x00\x89\xEC xxxxx?xx?xx????xx?xx?xxxx?xxxxxxxxx?xxxxxx?xxx?xx?xxx?xxxx?xx?xx
			Add("SetSize", "Portal 2 Build 7054",
				"VPanel::SetSize",
				"55 89 E5 83 EC ? 89 5D ? 8B 1D ? ? ? ? 89 75 ? 8B 45 ? 8B 13 8B 72 ? 8B 10 89 04 24 FF 12 8B 55 ? 89 1C 24 89 44 24 ? 89 54 24 ? 8B 55 ? 89 54 24 ? FF D6 8B 5D ? 8B 75 ? 89 EC");

			// \x55\x89\xE5\x83\xEC\x00\x8B\x45\x00\x83\xF8\x00\x74\x00\xC9 xxxxx?xx?xx?x?x
			Add("GetLocalPlayer", "Portal 2 Build 7054",
				"C_BasePlayer::GetLocalPlayer",
				"55 89 E5 83 EC ? 8B 45 ? 83 F8 ? 74 ? C9");

			// \x55\x89\xE5\x57\x89\xCF\x56\x89\xD6 xxxxxxxxx
			Add("GetPos", "Portal 2 Build 7054",
				"GetPos",
				"55 89 E5 57 89 CF 56 89 D6");

			// \x55\x89\xE5\x57\x56\x53\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x8B\x5D\x00\x8B\x78\x00\x85\xFF\x0F\x84\x00\x00\x00\x00 xxxxxxxx????x????xx?xx?xxxx????
			Add("g_pMatSystemSurface", "Portal 2 Build 7054",
				"CNetGraphPanel::DrawTextFields",
				"55 89 E5 57 56 53 81 EC ? ? ? ? A1 ? ? ? ? 8B 5D ? 8B 78 ? 85 FF 0F 84 ? ? ? ? ",
				281);

			// \x55\x89\xE5\x57\x56\x53\x81\xEC\x00\x00\x00\x00\x8B\x5D\x00\xA1\x00\x00\x00\x00\x8B\x8B\x00\x00\x00\x00 xxxxxxxx????xx?x????xx????
			Add("Paint", "Portal 2 Build 7054",
				"CFPSPanel::Paint",
				"55 89 E5 57 56 53 81 EC ? ? ? ? 8B 5D ? A1 ? ? ? ? 8B 8B ? ? ? ? ");

			// \x80\x3D\x00\x00\x00\x00\x00\x55\xB8\x00\x00\x00\x00\x89\xE5 xx?????xx????xx
			Add("ShouldDraw", "Portal 2 Build 7054",
				"CFPSPanel::ShouldDraw",
				"80 3D ? ? ? ? ? 55 B8 ? ? ? ? 89 E5");

			// \x55\x89\xE5\x57\x56\x53\x83\xEC\x00\x8B\x75\x00\x8B\x7D\x00\x8B\x46\x00\x85\xC0\x7E\x00\x31\xDB\xEB\x00\x8D\xB6\x00\x00\x00\x00\x83\xC3\x00\x39\x5E\x00\x7E\x00\x8B\x46\x00 xxxxxxxx?xx?xx?xx?xxx?xxx?xxxxxxxx?xx?x?xx?
			Add("FindElement", "Portal 2 Build 7054",
				"CHud::FindElement",
				"55 89 E5 57 56 53 83 EC ? 8B 75 ? 8B 7D ? 8B 46 ? 85 C0 7E ? 31 DB EB ? 8D B6 00 00 00 00 83 C3 ? 39 5E ? 7E ? 8B 46 ? ");


			// engine.so


			// \x55\x89\xE5\x56\x53\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x8B\x75\x00\x85\xC0 xxxxxxx????x????xx?xx
			Add("CvarPtr", "Portal 2 Build 7054",
				"Cvar",
				"55 89 E5 56 53 81 EC ? ? ? ? A1 ? ? ? ? 8B 75 ? 85 C0",
				100);

			// \x55\x89\xE5\x53\x83\xEC\x00\x8B\x5D\x00\x8B\x45\x00\xC6\x43\x00\x00\xC7\x43\x00\x00\x00\x00\x00\xC7\x43\x00\x00\x00\x00\x00\xC7\x43\x00\x00\x00\x00\x00\xC7\x43\x00\x00\x00\x00\x00\xC7\x03\x00\x00\x00\x00\xC7\x43\x00\x00\x00\x00\x00\xC7\x43\x00\x00\x00\x00\x00 xxxxxx?xx?xx?xx??xx?????xx?????xx?????xx?????xx????xx?????xx?????
			Add("ConVar_Ctor3", "Portal 2 Build 7054",
				"ConVar",
				"55 89 E5 53 83 EC ? 8B 5D ? 8B 45 ? C6 43 ? ? C7 43 ? ? ? ? ? C7 43 ? ? ? ? ? C7 43 ? ? ? ? ? C7 43 ? ? ? ? ? C7 03 ? ? ? ? C7 43 ? ? ? ? ? C7 43 ? ? ? ? ? ");

			// \x55\xB9\x00\x00\x00\x00\x89\xE5\x53\x83\xEC\x00\x8B\x5D\x00\x8B\x45\x00\x8B\x55\x00\xC6\x43\x00\x00\x89\x43\x00\x0F\xB6\x43\x00\x85\xD2 xx????xxxxx?xx?xx?xx?xx??xx?xxx?xx
			Add("ConCommand_Ctor1", "Portal 2 Build 7054",
				"ConCommand",
				"55 B9 ? ? ? ? 89 E5 53 83 EC ? 8B 5D ? 8B 45 ? 8B 55 ? C6 43 ? ? 89 43 ? 0F B6 43 ? 85 D2");

			// \x55\xB9\x00\x00\x00\x00\x89\xE5\x53\x83\xEC\x00\x8B\x5D\x00\x8B\x45\x00\x8B\x55\x00\xC6\x43\x00\x00\x89\x43\x00\x0F\xB6\x43\x00\xC7\x43\x00\x00\x00\x00\x00 xx????xxxxx?xx?xx?xx?xx??xx?xxx?xx?????
			Add("ConCommand_Ctor2", "Portal 2 Build 7054",
				"ConCommand",
				"55 B9 ? ? ? ? 89 E5 53 83 EC ? 8B 5D ? 8B 45 ? 8B 55 ? C6 43 ? ? 89 43 ? 0F B6 43 ? C7 43 ? ? ? ? ? ");
			
			// \x55\x89\xE5\x57\x56\x53\x83\xEC\x00\xE8\x00\x00\x00\x00\x8B\x10 xxxxxxxx?x????xx
			Add("engineClient", "Portal 2 Build 7054",
				"engineClient",
				"55 89 E5 57 56 53 83 EC ? E8 ? ? ? ? 8B 10",
				488);

			// \x55\x89\xE5\x83\xEC\x00\x8B\x45\x00\x85\xC0\x74\x00\x8B\x55\x00 xxxxx?xx?xxx?xx?
			Add("GetGameDir", "Portal 2 Build 7054",
				"CVEngineServer::GetGameDir",
				"55 89 E5 83 EC ? 8B 45 ? 85 C0 74 ? 8B 55 ? ");

			// \x55\x89\xE5\x57\x56\x53\x83\xEC\x00\x8B\x5D\x00\x8B\x7D\x00\x8B\x4D\x00 xxxxxxxx?xx?xx?xx?
			Add("curtime", "Portal 2 Build 7054",
				"CGlobalVarsBase::curtime",
				"55 89 E5 57 56 53 83 EC ? 8B 5D ? 8B 7D ? 8B 4D ? ",
				102);

			// \x55\x89\xE5\x57\x56\x53\x83\xEC\x00\x8B\x5D\x00\x89\x1C\x24\xE8\x00\x00\x00\x00\xC7\x04\x24\x00\x00\x00\x00\xE8\x00\x00\x00\x00 xxxxxxxx?xx?xxxx????xxx????x????
			Add("m_bLoadgame", "Portal 2 Build 7054",
				"m_bLoadgame",
				"55 89 E5 57 56 53 83 EC ? 8B 5D ? 89 1C 24 E8 ? ? ? ? C7 04 24 ? ? ? ? E8 ? ? ? ? ",
				34);

			// \x55\x89\xE5\x57\x56\x53\x81\xEC\x00\x00\x00\x00\xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00 xxxxxxxx????xx????????xx????
			Add("m_szMapname", "Portal 2 Build 7054",
				"CBaseServer::m_szMapname[64]",
				"55 89 E5 57 56 53 81 EC ? ? ? ? C7 05 ? ? ? ? ? ? ? ? 8B 0D ? ? ? ? ",
				99);

			// \x55\x89\xE5\x57\x56\x53\x81\xEC\x00\x00\x00\x00\x8B\x4D\x00\x8B\x55\x00\x8B\x7D\x00 xxxxxxxx????xx?xx?xx?
			Add("m_currentState", "Portal 2 Build 7054",
				"CHostState::m_currentState",
				"55 89 E5 57 56 53 81 EC ? ? ? ? 8B 4D ? 8B 55 ? 8B 7D ? ",
				147);

			// \x55\x89\xE5\x83\xEC\x00\x89\x5D\x00\x89\xC3\xA1\x00\x00\x00\x00\x89\x75\x00\x89\xD6\x85\xC0 xxxxx?xx?xxx????xx?xxxx
			Add("demorecorder", "Portal 2 Build 7054",
				"demorecorder",
				"55 89 E5 83 EC ? 89 5D ? 89 C3 A1 ? ? ? ? 89 75 ? 89 D6 85 C0",
				26);

			// \x55\x89\xE5\x53\x83\xEC\x00\x8B\x5D\x00\x83\x3B\x00\x75\x00\xA1\x00\x00\x00\x00 xxxxxx?xx?xx?x?x????
			Add("g_pInputSystem", "Portal 2 Build 7054",
				"g_pInputSystem",
				"55 89 E5 53 83 EC ? 8B 5D ? 83 3B ? 75 ? A1 ? ? ? ? ",
				16);

			// \x55\x89\xE5\x83\xEC\x00\x8B\x45\x00\x89\x75\x00\x89\x5D\x00\x8B\x75\x00\x89\x7D\x00\x83\xF8\x00\x0F\x84\x00\x00\x00\x00 xxxxx?xx?xx?xx?xx?xx?xx?xx????
			Add("Key_SetBinding", "Portal 2 Build 7054",
				"Key_SetBinding",
				"55 89 E5 83 EC ? 8B 45 ? 89 75 ? 89 5D ? 8B 75 ? 89 7D ? 83 F8 ? 0F 84 ? ? ? ? ");

			// \x55\x89\xE5\x83\xEC\x00\x89\x5D\x00\x89\xC3\xA1\x00\x00\x00\x00\x89\x75\x00\x89\xD6\x85\xC0 xxxxx?xx?xxx????xx?xxxx
			Add("demoplayer", "Portal 2 Build 7054",
				"demoplayer",
				"55 89 E5 83 EC ? 89 5D ? 89 C3 A1 ? ? ? ? 89 75 ? 89 D6 85 C0",
				43);

			// \x55\x89\xE5\x83\xEC\x00\x89\x5D\x00\x8B\x5D\x00\x89\x75\x00\x8B\x75\x00\x89\x7D\x00\x8B\x7D\x00\x83\xFB\x00\x0F\x84\x00\x00\x00\x00 xxxxx?xx?xx?xx?xx?xx?xx?xx?xx????
			Add("SetSignonState", "Portal 2 Build 7054",
				"CGameClient::SetSignonState",
				"55 89 E5 83 EC ? 89 5D ? 8B 5D ? 89 75 ? 8B 75 ? 89 7D ? 8B 7D ? 83 FB ? 0F 84 ? ? ? ? ");

			// \x55\x89\xE5\x56\x53\x83\xEC\x00\x8B\x5D\x00\x8D\x73\x00\x89\x34\x24 xxxxxxx?xx?xx?xxx
			Add("CloseDemoFile", "Portal 2 Build 7054",
				"CDemoRecorder::CloseDemoFile",
				"55 89 E5 56 53 83 EC ? 8B 5D ? 8D 73 ? 89 34 24");

			// \x55\x89\xE5\x53\x83\xEC\x00\x8B\x5D\x00\x8B\x03\x89\x1C\x24\xFF\x50\x00\x84\xC0\x0F\x84\x00\x00\x00\x00 xxxxxx?xx?xxxxxxx?xxxx????
			Add("StopRecording", "Portal 2 Build 7054",
				"CDemoRecorder::StopRecording",
				"55 89 E5 53 83 EC ? 8B 5D ? 8B 03 89 1C 24 FF 50 ? 84 C0 0F 84 ? ? ? ? ");

			// \x55\x89\xE5\x81\xEC\x00\x00\x00\x00\x89\x5D\x00\x8B\x5D\x00\x89\x75\x00\x89\x7D\x00\x80\xBB\x00\x00\x00\x00\x00 xxxxx????xx?xx?xx?xx?xx?????
			Add("StartupDemoFile", "Portal 2 Build 7054",
				"CDemoRecorder::StartupDemoFile",
				"55 89 E5 81 EC ? ? ? ? 89 5D ? 8B 5D ? 89 75 ? 89 7D ? 80 BB ? ? ? ? ? ");

			// \x55\x89\xE5\x83\xEC\x00\x83\x3D\x00\x00\x00\x00\x00\x74\x00\xC9\xC3\x8D\xB4\x26\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x8B\x10\x89\x04\x24\xFF\x52\x00\x84\xC0 xxxxx?xx?????x?xxxxxxxxxx????xxxxxxx?xx
			Add("ConCommandStop", "Portal 2 Build 7054",
				"CON_COMMAND stop",
				"55 89 E5 83 EC ? 83 3D ? ? ? ? ? 74 ? C9 C3 8D B4 26 00 00 00 00 A1 ? ? ? ? 8B 10 89 04 24 FF 52 ? 84 C0");

			// \x55\x89\xE5\x56\x53\x83\xEC\x00\x8B\x5D\x00\xF2\x0F\x10\x05\x00\x00\x00\x00 xxxxxxx?xx?xxxx????
			Add("Disconnect", "Portal 2 Build 7054",
				"CBaseClientState::Disconnect",
				"55 89 E5 56 53 83 EC ? 8B 5D ? F2 0F 10 05 ? ? ? ? ");

			// \x55\x89\xE5\x56\x53\x81\xEC\x00\x00\x00\x00\x8B\x15\x00\x00\x00\x00\x89\x15\x00\x00\x00\x00 xxxxxxx????xx????xx????
			Add("PlayDemo", "Portal 2 Build 7054",
				"CL_PlayDemo_f",
				"55 89 E5 56 53 81 EC ? ? ? ? 8B 15 ? ? ? ? 89 15 ? ? ? ? ");

			// \x55\x89\xE5\x57\x56\x53\x83\xEC\x00\x0F\xB6\x45\x00\xC7\x04\x24\x00\x00\x00\x00\x8B\x5D\x00 xxxxxxxx?xxx?xxx????xx?
			Add("StartPlayback", "Portal 2 Build 7054",
				"CDemoPlayer::StartPlayback",
				"55 89 E5 57 56 53 83 EC ? 0F B6 45 ? C7 04 24 ? ? ? ? 8B 5D ? ");

			// \x55\x89\xE5\x83\xEC\x00\x8B\x45\x00\xC7\x04\x24\x00\x00\x00\x00\x89\x44\x24\x00\xE8\x00\x00\x00\x00\xC9\xC3\x90\x55 xxxxx?xx?xxx????xxx?x????xxxx
			Add("HostStateFrame", "Portal 2 Build 7054",
				"HostState_Frame",
				"55 89 E5 83 EC ? 8B 45 ? C7 04 24 ? ? ? ? 89 44 24 ? E8 ? ? ? ? C9 C3 90 55");

			// \x55\x89\xE5\x57\x56\x53\x81\xEC\x00\x00\x00\x00\xC6\x85\x00\x00\x00\x00\x00\x8B\x5D\x00 xxxxxxxx????xx?????xx?
			Add("PrintDescription", "Portal 2 Build 7054",
				"ConVar_PrintDescription",
				"55 89 E5 57 56 53 81 EC ? ? ? ? C6 85 ? ? ? ? ? 8B 5D ? ",
				779);

			// \x55\x89\xE5\x57\x56\x53\x81\xEC\x00\x00\x00\x00\xC7\x45\x00\x00\x00\x00\x00\x8B\x5D\x00\xC6\x45\x00\x00\x8D\x43\x00 xxxxxxxx????xx?????xx?xx??xx?
			Add("ReadPacket", "Portal 2 Build 7054",
				"CDemoPlayer::ReadPacket",
				"55 89 E5 57 56 53 81 EC ? ? ? ? C7 45 ? ? ? ? ? 8B 5D ? C6 45 ? ? 8D 43 ? ",
				1407);
		}
	}
}