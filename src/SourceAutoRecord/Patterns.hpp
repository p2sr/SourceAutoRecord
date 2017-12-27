#pragma once
#include "Utils.hpp"

namespace Patterns
{
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
		auto pattern = Get(patternName);

		(*pattern).Signatures.push_back(Signature
		{
			version,
			sigName,
			sigBytes,
			offset
		});
	}
	void Add(const char* patternName, const char* version, const char* sigName, const int index = 0)
	{
		auto pattern = Get(patternName);

		(*pattern).Signatures.push_back(Signature
		{
			version,
			sigName,
			(*pattern).Signatures[index].Bytes,
			(*pattern).Signatures[index].Offset,
		});
	}
	void Init()
	{
		Create("server.dll", "CheckJumpButton");
		Create("server.dll", "PlayerUse");
		Create("server.dll", "AirMove");
		Create("server.dll", "PlayerRunCommand");

		// \x55\x8B\xEC\x83\xEC\x0C\x56\x8B\xF1\x8B\x4E\x04 xxxxxxxxxxxx
		Add("CheckJumpButton", "Portal 2 Build 6879",
			"CPortalGameMovement::CheckJumpButton",
			"55 8B EC 83 EC 0C 56 8B F1 8B 4E 04");

		// \x55\x8B\xEC\x83\xEC\x18\x56\x8B\xF1\x8B\x4E\x04 xxxxxxxxxxxx
		Add("CheckJumpButton", "INFRA Build 6905",
			"CINFRAGameMovement::CheckJumpButton",
			"55 8B EC 83 EC 18 56 8B F1 8B 4E 04");

		// \x55\x8B\xEC\x8B\x15\x00\x00\x00\x00\x83\xEC\x0C\x56\x8B\xF1 xxxxx????xxxxxx
		Add("PlayerUse", "Portal 2 Build 6879",
			"CPortal_Player::PlayerUse",
			"55 8B EC 8B 15 ? ? ? ? 83 EC 0C 56 8B F1");

		// \x57\x8B\xF9\x8B\x87\x00\x00\x00\x00\x0B\x87\x00\x00\x00\x00 xxxxx????xx????
		Add("PlayerUse", "INFRA Build 6905",
			"CINFRA_Player::PlayerUse",
			"57 8B F9 8B 87 ? ? ? ? 0B 87 ? ? ? ? ");

		// \x55\x8B\xEC\x83\xEC\x50\x56\x8B\xF1\x8D\x45\xBC xxxxxxxxxxxx
		Add("AirMove", "Portal 2 Build 6879",
			"CPortalGameMovement::AirMove",
			"55 8B EC 83 EC 50 56 8B F1 8D 45 BC",
			679);
		// \x55\x8B\xEC\x83\xEC\x50\x56\x8B\xF1\x8D\x45\xBC xxxxxxxxxxxx
		Add("AirMove", "INFRA Build 6905",
			"CINFRAGameMovement::AirMove",
			"55 8B EC 83 EC 50 56 8B F1 8D 45 BC",
			689);

		// \x55\x8B\xEC\x56\x8B\x75\x08\x57\x8B\xF9\x33\xC0 xxxxxxxxxxxx
		Add("PlayerRunCommand", "Portal 2 Build 6879",
			"CBasePlayer::PlayerRunCommand",
			"55 8B EC 56 8B 75 08 57 8B F9 33 C0",
			162);

		Create("client.dll", "Paint");
		Create("client.dll", "SetSize");
		Create("client.dll", "ShouldDraw");
		Create("client.dll", "g_pMatSystemSurface");
		Create("client.dll", "FindElement");

		// \x53\x8B\xDC\x83\xEC\x08\x83\xE4\xF0\x83\xC4\x04\x55\x8B\x6B\x04\x89\x6C\x24\x04\x8B\xEC\x81\xEC\x00\x00\x00\x00\x56\x8B\xF1\x8B\x0D\x00\x00\x00\x00\x8B\x01\x8B\x96\x00\x00\x00\x00 xxxxxxxxxxxxxxxxxxxxxxxx????xxxxx????xxxx????
		Add("Paint", "Portal 2 Build 6879",
			"CFPSPanel::Paint",
			"53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 56 8B F1 8B 0D ? ? ? ? 8B 01 8B 96 ? ? ? ? ");

		// \x53\x8B\xDC\x83\xEC\x08\x83\xE4\xF0\x83\xC4\x04\x55\x8B\x6B\x04\x89\x6C\x24\x04\x8B\xEC\x81\xEC\x00\x00\x00\x00\x56\x57\x8B\xF9\x8B\x0D\x00\x00\x00\x00\x8B\x01\x8B\x97\x00\x00\x00\x00 xxxxxxxxxxxxxxxxxxxxxxxx????xxxxxx????xxxx????
		Add("Paint", "INFRA Build 6905",
			"CFPSPanel::Paint",
			"53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 56 57 8B F9 8B 0D ? ? ? ? 8B 01 8B 97 ? ? ? ? ");

		// \x55\x8B\xEC\x8B\x41\x04\x8B\x50\x04\x8B\x45\x0C\x56\x8B\x35\x00\x00\x00\x00\x57\x8B\x3E\x8D\x4C\x0A\x04\x8B\x55\x08\x50\x8B\x01\x52\x8B\x10\xFF\xD2\x50\x8B\x47\x10 xxxxxxxxxxxxxxx????xxxxxxxxxxxxxxxxxxxxxx

		Add("SetSize", "Portal 2 Build 6879",
			"VPanel::SetSize",
			"55 8B EC 8B 41 04 8B 50 04 8B 45 0C 56 8B 35 ? ? ? ? 57 8B 3E 8D 4C 0A 04 8B 55 08 50 8B 01 52 8B 10 FF D2 50 8B 47 10");
		Add("SetSize", "INFRA Build 6905",
			"VPanel::SetSize");

		// \x80\x3D\x00\x00\x00\x00\x00\x75\x7C xx?????xx
		Add("ShouldDraw", "Portal 2 Build 6879",
			"CFPSPanel::ShouldDraw",
			"80 3D ? ? ? ? ? 75 7C");

		// \x80\x3D\x00\x00\x00\x00\x00\x56\x8B\xF1\x0F\x85\x00\x00\x00\x00 xx?????xxxxx????
		Add("ShouldDraw", "INFRA Build 6905",
			"CFPSPanel::ShouldDraw",
			"80 3D ? ? ? ? ? 56 8B F1 0F 85 ? ? ? ? ");
		
		// \x55\x8B\xEC\xA1\x00\x00\x00\x00\x81\xEC\x00\x00\x00\x00\x83\x78\x30\x00\x56\x8B\xF1\x0F\x84\x00\x00\x00\x00 xxxx????xx????xxxxxxxxx????
		Add("g_pMatSystemSurface", "Portal 2 Build 6879",
			"CNetGraphPanel::DrawTextFields",
			"55 8B EC A1 ? ? ? ? 81 EC ? ? ? ? 83 78 30 00 56 8B F1 0F 84 ? ? ? ? ",
			264);
		
		// \x55\x8B\xEC\xA1\x00\x00\x00\x00\x8B\x50\x34\x81\xEC\x00\x00\x00\x00\x56\x8B\xF1\xB9\x00\x00\x00\x00\xFF\xD2\x85\xC0\x0F\x84\x00\x00\x00\x00 xxxx????xxxxx????xxxx????xxxxxx????
		Add("g_pMatSystemSurface", "INFRA Build 6905",
			"CNetGraphPanel::DrawTextFields",
			"55 8B EC A1 ? ? ? ? 8B 50 34 81 EC ? ? ? ? 56 8B F1 B9 ? ? ? ? FF D2 85 C0 0F 84 ? ? ? ? ",
			278);
		
		// \x55\x8B\xEC\x53\x8B\x5D\x08\x56\x57\x8B\xF1\x33\xFF\x39\x7E\x28 xxxxxxxxxxxxxxxx
		Add("FindElement", "Portal 2 Build 6879",
			"CHud::FindElement",
			"55 8B EC 53 8B 5D 08 56 57 8B F1 33 FF 39 7E 28");

		Create("engine.dll", "ConVar_Ctor3");
		Create("engine.dll", "CvarPtr");
		Create("engine.dll", "g_pInputSystem");
		Create("engine.dll", "Key_SetBinding");
		Create("engine.dll", "ConCommand_Ctor1");
		Create("engine.dll", "ConCommand_Ctor2");
		Create("engine.dll", "SetSignonState");
		Create("engine.dll", "OnFileSelected");
		Create("engine.dll", "GetGameDir");
		Create("engine.dll", "engineClient");
		Create("engine.dll", "m_bLoadgame");
		Create("engine.dll", "curtime");
		Create("engine.dll", "m_szMapname");
		Create("engine.dll", "CloseDemoFile");
		Create("engine.dll", "demorecorder");
		Create("engine.dll", "demoplayer");
		Create("engine.dll", "PlayDemo");
		Create("engine.dll", "StartPlayback");
		Create("engine.dll", "StopRecording");
		Create("engine.dll", "StartupDemoFile");
		Create("engine.dll", "ReadPacket");
		Create("engine.dll", "Stop");
		Create("engine.dll", "PrintDescription");
		Create("engine.dll", "Disconnect");
		Create("engine.dll", "HostStateFrame");
		Create("engine.dll", "m_currentState");

		// \x55\x8B\xEC\xF3\x0F\x10\x45\x00\x8B\x55\x14 xxxxxxx?xxx
		Add("ConVar_Ctor3", "Portal 2 Build 6879",
			"ConVar",
			"55 8B EC F3 0F 10 45 ? 8B 55 14");
		Add("ConVar_Ctor3", "INFRA Build 6905", "ConVar");

		// \x8B\x0D\x00\x00\x00\x00\x8B\x01\x8B\x50\x40\x68\x00\x00\x00\x00\xFF\xD2\x85\xC0\x74\x17 xx????xxxxxx????xxxxxxx
		Add("CvarPtr", "Portal 2 Build 6879",
			"Cvar",
			"8B 0D ? ? ? ? 8B 01 8B 50 40 68 ? ? ? ? FF D2 85 C0 74 17", 2);
		Add("CvarPtr", "INFRA Build 6905", "Cvar");

		// \x55\x8B\xEC\x56\x8B\x75\x08\x83\x3E\x02\x74\x11\x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x83\xC4\x04\x5E\x5D\xC3\x8B\x86\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00 xxxxxxxxxxxxx????xx????xxxxxxxx????xx????
		Add("g_pInputSystem", "Portal 2 Build 6879",
			"CON_COMMAND unbind",
			"55 8B EC 56 8B 75 08 83 3E 02 74 11 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 04 5E 5D C3 8B 86 ? ? ? ? 8B 0D ? ? ? ? ", 37);
		Add("g_pInputSystem", "INFRA Build 6905", "CON_COMMAND unbind");
		
		// \x55\x8B\xEC\x56\x8B\x75\x08\x83\xFE\xFF xxxxxxxxxx
		Add("Key_SetBinding", "Portal 2 Build 6879",
			"Key_SetBinding",
			"55 8B EC 56 8B 75 08 83 FE FF");
		
		// \x55\x8B\xEC\x57\x8B\x7D\x08\x83\xFF\xFF xxxxxxxxxx
		Add("Key_SetBinding", "INFRA Build 6905",
			"Key_SetBinding",
			"55 8B EC 57 8B 7D 08 83 FF FF");
		
		// \x55\x8B\xEC\x8B\x45\x0C\x53\x33\xDB\x56\x8B\xF1\x8B\x4D\x18\x80\x66\x20\xF9 xxxxxxxxxxxxxxxxxxx
		Add("ConCommand_Ctor1", "Portal 2 Build 6879",
			"ConCommand",
			"55 8B EC 8B 45 0C 53 33 DB 56 8B F1 8B 4D 18 80 66 20 F9");
		Add("ConCommand_Ctor1", "INFRA Build 6905", "ConCommand");
		
		// \x55\x8B\xEC\x8B\x45\x0C\x53\x33\xDB\x56\x8B\xF1\x8B\x4D\x18\x80\x4E\x20\x02 xxxxxxxxxxxxxxxxxxx
		Add("ConCommand_Ctor2", "Portal 2 Build 6879",
			"ConCommand",
			"55 8B EC 8B 45 0C 53 33 DB 56 8B F1 8B 4D 18 80 4E 20 02");
		Add("ConCommand_Ctor2", "INFRA Build 6905", "ConCommand");
		
		// \x55\x8B\xEC\x56\x57\x8B\x7D\x08\x8B\xF1\x83\xFF\x02 xxxxxxxxxxxxx
		Add("SetSignonState", "Portal 2 Build 6879",
			"CGameClient::SetSignonState",
			"55 8B EC 56 57 8B 7D 08 8B F1 83 FF 02");
		Add("SetSignonState", "INFRA Build 6905", "CGameClient::SetSignonState");
		
		// \x55\x8B\xEC\x8B\x45\x08\x81\xEC\x00\x00\x00\x00\x56\x8B\xF1\x85\xC0\x0F\x84\x00\x00\x00\x00\x80\x38\x00\x0F\x84\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x8B\x11\x57\x68\x00\x00\x00\x00\x8D\xBD\x00\x00\x00\x00\x57\x50\x8B\x82\x00\x00\x00\x00\xFF\xD0\x6A\x0A xxxxxxxx????xxxxxxx????xxxxx????xx????xxxx????xx????xxxx????xxxx
		Add("OnFileSelected", "Portal 2 Build 6879",
			"CDemoUIPanel::OnFileSelected",
			"55 8B EC 8B 45 08 81 EC ? ? ? ? 56 8B F1 85 C0 0F 84 ? ? ? ? 80 38 00 0F 84 ? ? ? ? 8B 0D ? ? ? ? 8B 11 57 68 ? ? ? ? 8D BD ? ? ? ? 57 50 8B 82 ? ? ? ? FF D0 6A 0A");
		Add("OnFileSelected", "INFRA Build 6905", "CDemoUIPanel::OnFileSelected");
		
		// \x55\x8B\xEC\x8B\x45\x08\x85\xC0\x74\x12\x8B\x4D\x0C xxxxxxxxxxxxx
		Add("GetGameDir", "Portal 2 Build 6879",
			"CVEngineServer::GetGameDir",
			"55 8B EC 8B 45 08 85 C0 74 12 8B 4D 0C");
		Add("GetGameDir", "INFRA Build 6905", "CVEngineServer::GetGameDir");
		
		// \x55\x8B\xEC\x83\xEC\x10\xB8\x00\x00\x00\x00\x99 xxxxxxx????x
		Add("engineClient", "Portal 2 Build 6879",
			"engineClient",
			"55 8B EC 83 EC 10 B8 ? ? ? ? 99",
			63);
		
		// \x55\x8B\xEC\x83\xEC\x14\xB8\x00\x00\x00\x00 xxxxxxx????
		Add("engineClient", "INFRA Build 6905",
			"engineClient",
			"55 8B EC 83 EC 14 B8 ? ? ? ? ",
			96);
		
		// \x55\x8B\xEC\x83\xEC\x14\x80\x3D\x00\x00\x00\x00\x00\x56 xxxxxxxx?????x
		Add("m_bLoadgame", "Portal 2 Build 6879",
			"CGameClient::SpawnPlayer",
			"55 8B EC 83 EC 14 80 3D ? ? ? ? ? 56",
			8);
		Add("m_bLoadgame", "INFRA Build 6905", "CGameClient::SpawnPlayer");
		
		// \x89\x96\xC4\x00\x00\x00\x8B\x86\xC8\x00\x00\x00\x8B\xCE\xA3\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xD9\x1D\x00\x00\x00\x00\x8B\xCE\xE8\x00\x00\x00\x00\xD9\x1D xxxxxxxxxxxxxxx????x????xx????xxx????xx
		Add("curtime", "Portal 2 Build 6879",
			"CGlobalVarsBase::curtime",
			"89 96 C4 00 00 00 8B 86 C8 00 00 00 8B CE A3 ? ? ? ? E8 ? ? ? ? D9 1D ? ? ? ? 8B CE E8 ? ? ? ? D9 1D", 26);
		Add("curtime", "INFRA Build 6905", "CGlobalVarsBase::curtime");
		
		// \xD9\x00\x2C\xD9\xC9\xDF\xF1\xDD\xD8\x76\x00\x80\x00\x00\x00\x00\x00\x00 x?xxxxxxxx?x?????x
		Add("m_szMapname", "Portal 2 Build 6879",
			"CBaseServer::m_szMapname[64]",
			"D9 ? 2C D9 C9 DF F1 DD D8 76 ? 80 ? ? ? ? ? 00",
			13);
		
		// \x76\x50\x80\x3D\x00\x00\x00\x00\x00\xB8\x00\x00\x00\x00 xxxx?????x????
		Add("m_szMapname", "INFRA Build 6905",
			"CBaseServer::m_szMapname[64]",
			"76 50 80 3D ? ? ? ? ? B8 ? ? ? ? ",
			10);
		
		// \x56\x8B\xF1\x57\x8D\x4E\x04\xE8\x00\x00\x00\x00\x84\xC0\x0F\x84\x00\x00\x00\x00\x80\xBE\x00\x00\x00\x00\x00 xxxxxxxx????xxxx????xx?????
		Add("CloseDemoFile", "Portal 2 Build 6879",
			"CDemoRecorder::CloseDemoFile",
			"56 8B F1 57 8D 4E 04 E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 80 BE ? ? ? ? ? ");
		Add("CloseDemoFile", "INFRA Build 6905", "CDemoRecorder::CloseDemoFile");
		
		// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x83\x3D\x00\x00\x00\x00\x00\x53 xxxxx????xx?????x
		Add("demorecorder", "Portal 2 Build 6879",
			"demorecorder",
			"55 8B EC 81 EC ? ? ? ? 83 3D ? ? ? ? ? 53",
			43);
		Add("demorecorder", "INFRA Build 6905", "demorecorder");
		
		// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x83\x3D\x00\x00\x00\x00\x00\x53 xxxxx????xx?????x
		Add("demoplayer", "Portal 2 Build 6879",
			"demoplayer",
			"55 8B EC 81 EC ? ? ? ? 83 3D ? ? ? ? ? 53",
			79);
		Add("demoplayer", "INFRA Build 6905", "demoplayer");

		// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x83\x3D\x00\x00\x00\x00\x00\x0F\x85\x00\x00\x00\x00\x8B\x45\x08\x83\x38\x02 xxxxx????xx?????xx????xxxxxx
		Add("PlayDemo", "Portal 2 Build 6879",
			"CL_PlayDemo_f",
			"55 8B EC 81 EC ? ? ? ? 83 3D ? ? ? ? ? 0F 85 ? ? ? ? 8B 45 08 83 38 02");
		Add("PlayDemo", "INFRA Build 6905", "CL_PlayDemo_f");

		// \x55\x8B\xEC\x53\x56\x57\x6A\x00\x8B\xF1 xxxxxxxxxx
		Add("StartPlayback", "Portal 2 Build 6879",
			"CDemoPlayer::StartPlayback",
			"55 8B EC 53 56 57 6A 00 8B F1");
		Add("StartPlayback", "INFRA Build 6905", "CDemoPlayer::StartPlayback");

		// \x55\x8B\xEC\x51\x56\x8B\xF1\x8B\x06\x8B\x50\x10 xxxxxxxxxxxx
		Add("StopRecording", "Portal 2 Build 6879",
			"CDemoRecorder::StopRecording",
			"55 8B EC 51 56 8B F1 8B 06 8B 50 10");

		// \x56\x8B\xF1\x8B\x06\x8B\x50\x10\xFF\xD2\x84\xC0\x74\x63 xxxxxxxxxxxxxx
		Add("StopRecording", "INFRA Build 6905",
			"CDemoRecorder::StopRecording",
			"56 8B F1 8B 06 8B 50 10 FF D2 84 C0 74 63");

		// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x53\x8B\xD9\x80\xBB\x00\x00\x00\x00\x00\x56 xxxxx????xxxxx?????x
		Add("StartupDemoFile", "Portal 2 Build 6879",
			"CDemoRecorder::StartupDemoFile",
			"55 8B EC 81 EC ? ? ? ? 53 8B D9 80 BB ? ? ? ? ? 56");
		Add("StartupDemoFile", "INFRA Build 6905", "CDemoRecorder::StartupDemoFile");

		// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x57\x8B\xF9\x8D\x4F\x04\x89\x7D\xB0 xxxxx????xxxxxxxxx
		Add("ReadPacket", "Portal 2 Build 6879",
			"CDemoPlayer::ReadPacket",
			"55 8B EC 81 EC ? ? ? ? 57 8B F9 8D 4F 04 89 7D B0",
			414);
		Add("ReadPacket", "INFRA Build 6905", "CDemoPlayer::ReadPacket");

		// \x83\x3D\x00\x00\x00\x00\x00\x75\x1F xx?????xx
		Add("Stop", "Portal 2 Build 6879",
			"CON_COMMAND stop",
			"83 3D ? ? ? ? ? 75 1F");

		// \x83\x3D\x00\x00\x00\x00\x00\x75\x1F\x8B\x0D\x00\x00\x00\x00\x8B\x01 xx?????xxxx????xx
		Add("Stop", "INFRA Build 6905",
			"CON_COMMAND stop",
			"83 3D ? ? ? ? ? 75 1F 8B 0D ? ? ? ? 8B 01");

		// \x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x83\xC4\x0C\x8B\xE5\x5D\xC3\x8D\x8D\x00\x00\x00\x00 x????xx????xxxxxxxxx????
		Add("PrintDescription", "Portal 2 Build 6879",
			"ConVar_PrintDescription",
			"68 ? ? ? ? FF 15 ? ? ? ? 83 C4 0C 8B E5 5D C3 8D 8D ? ? ? ? ",
			1);
		Add("PrintDescription", "INFRA Build 6905",
			"ConVar_PrintDescription");

		// \x55\x8B\xEC\xDD\x05\x00\x00\x00\x00\x83\xEC\x18 xxxxx????xxx
		Add("Disconnect", "Portal 2 Build 6879",
			"CBaseClientState::Disconnect",
			"55 8B EC DD 05 ? ? ? ? 83 EC 18");

		// \x55\x8B\xEC\xF2\x0F\x10\x05\x00\x00\x00\x00 xxxxxxx????
		Add("Disconnect", "INFRA Build 6905",
			"CBaseClientState::Disconnect",
			"55 8B EC F2 0F 10 05 ? ? ? ? ");

		// \x55\x8B\xEC\xD9\x45\x08\x51\xB9\x00\x00\x00\x00\xD9\x1C\x24\xE8\x00\x00\x00\x00\x5D\xC3 xxxxxxxx????xxxx????xx
		Add("HostStateFrame", "Portal 2 Build 6879",
			"HostState_Frame",
			"55 8B EC D9 45 08 51 B9 ? ? ? ? D9 1C 24 E8 ? ? ? ? 5D C3");

		// \x55\x8B\xEC\xF3\x0F\x10\x45\x00\x51\xB9\x00\x00\x00\x00\xF3\x0F\x11\x04\x24\xE8\x00\x00\x00\x00\x5D\xC3 xxxxxxx?xx????xxxxxx????xx
		Add("HostStateFrame", "INFRA Build 6905",
			"HostState_Frame",
			"55 8B EC F3 0F 10 45 ? 51 B9 ? ? ? ? F3 0F 11 04 24 E8 ? ? ? ? 5D C3");

		// \xC7\x05\x00\x00\x00\x00\x07\x00\x00\x00\xC3 xx????xxxxx
		Add("m_currentState", "Portal 2 Build 6879",
			"CHostState::m_currentState",
			"C7 05 ? ? ? ? 07 00 00 00 C3",
			2);
		Add("m_currentState", "INFRA Build 6905", "CHostState::m_currentState");
	}
}