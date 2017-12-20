#pragma once
#include "Utils.hpp"

namespace Patterns
{
	std::vector<Pattern> Items;

	void Add(const char* name, const char* moduleName, const char* comment, const char* sigBytes, int offset = 0)
	{
		auto sig = Signature{
			comment,
			sigBytes,
			offset
		};

		for (auto &pattern : Items) {
			if (pattern.Name == name) {
				pattern.Signatures.push_back(sig);
				return;
			}
		}

		Items.push_back(Pattern{
			name,
			moduleName,
			std::vector<Signature>{ sig }
		});
	}
	Pattern Get(const char* name)
	{
		for (auto &pattern : Items) {
			if (pattern.Name == name) {
				return pattern;
			}
		}
	}
	void LoadAll()
	{
		// server.dll


		// CPortalGameMovement::CheckJumpButton
		// \x55\x8B\xEC\x83\xEC\x0C\x56\x8B\xF1\x8B\x4E\x04 xxxxxxxxxxxx
		Add("CheckJumpButton", "server.dll", "Portal 2 Build 6879", "55 8B EC 83 EC 0C 56 8B F1 8B 4E 04");

		// CINFRAGameMovement::CheckJumpButton
		// \x55\x8B\xEC\x83\xEC\x18\x56\x8B\xF1\x8B\x4E\x04 xxxxxxxxxxxx
		Add("CheckJumpButton", "server.dll", "INFRA Build 6905", "55 8B EC 83 EC 0C 56 8B F1 8B 4E 04");

		// CPortal_Player::PlayerUse
		// \x55\x8B\xEC\x8B\x15\x00\x00\x00\x00\x83\xEC\x0C\x56\x8B\xF1 xxxxx????xxxxxx
		Add("PlayerUse", "server.dll", "Portal 2 Build 6879", "55 8B EC 8B 15 ? ? ? ? 83 EC 0C 56 8B F1");

		// CPortalGameMovement::AirMove
		// \x55\x8B\xEC\x83\xEC\x50\x56\x8B\xF1\x8D\x45\xBC xxxxxxxxxxxx
		Add("AirMove", "server.dll", "Portal 2 Build 6879", "55 8B EC 83 EC 50 56 8B F1 8D 45 BC", 679);

		// CBasePlayer:PlayerRunCommand
		// \x55\x8B\xEC\x56\x8B\x75\x08\x57\x8B\xF9\x33\xC0 xxxxxxxxxxxx
		Add("PlayerRunCommand", "server.dll", "Portal 2 Build 6879", "55 8B EC 56 8B 75 08 57 8B F9 33 C0", 162);


		// client.dll


		// CFPSPanel::Paint
		// \x53\x8B\xDC\x83\xEC\x08\x83\xE4\xF0\x83\xC4\x04\x55\x8B\x6B\x04\x89\x6C\x24\x04\x8B\xEC\x81\xEC\x00\x00\x00\x00\x56\x8B\xF1\x8B\x0D\x00\x00\x00\x00\x8B\x01\x8B\x96\x00\x00\x00\x00 xxxxxxxxxxxxxxxxxxxxxxxx????xxxxx????xxxx????
		Add("Paint", "client.dll", "Portal 2 Build 6879", "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 56 8B F1 8B 0D ? ? ? ? 8B 01 8B 96 ? ? ? ? ");

		// VPanel::SetSize
		// \x55\x8B\xEC\x8B\x41\x04\x8B\x50\x04\x8B\x45\x0C\x56\x8B\x35\x00\x00\x00\x00\x57\x8B\x3E\x8D\x4C\x0A\x04\x8B\x55\x08\x50\x8B\x01\x52\x8B\x10\xFF\xD2\x50\x8B\x47\x10 xxxxxxxxxxxxxxx????xxxxxxxxxxxxxxxxxxxxxx
		Add("SetSize", "client.dll", "Portal 2 Build 6879", "55 8B EC 8B 41 04 8B 50 04 8B 45 0C 56 8B 35 ? ? ? ? 57 8B 3E 8D 4C 0A 04 8B 55 08 50 8B 01 52 8B 10 FF D2 50 8B 47 10");

		// CFPSPanel::ShouldDraw
		// \x80\x3D\x00\x00\x00\x00\x00\x75\x7C xx?????xx
		Add("ShouldDraw", "client.dll", "Portal 2 Build 6879", "80 3D ? ? ? ? ? 75 7C");

		// CNetGraphPanel::DrawTextFields
		// \x55\x8B\xEC\xA1\x00\x00\x00\x00\x81\xEC\x00\x00\x00\x00\x83\x78\x30\x00\x56\x8B\xF1\x0F\x84\x00\x00\x00\x00 xxxx????xx????xxxxxxxxx????
		Add("g_pMatSystemSurface", "client.dll", "Portal 2 Build 6879", "55 8B EC A1 ? ? ? ? 81 EC ? ? ? ? 83 78 30 00 56 8B F1 0F 84 ? ? ? ? ", 264);

		// CHud::FindElement
		// \x55\x8B\xEC\x53\x8B\x5D\x08\x56\x57\x8B\xF1\x33\xFF\x39\x7E\x28 xxxxxxxxxxxxxxxx
		Add("FindElement", "client.dll", "Portal 2 Build 6879", "55 8B EC 53 8B 5D 08 56 57 8B F1 33 FF 39 7E 28");


		// engine.dll


		// \x55\x8B\xEC\xF3\x0F\x10\x45\x00\x8B\x55\x14 xxxxxxx?xxx
		Add("ConVar_Ctor3", "engine.dll", "Portal 2 Build 6879", "55 8B EC F3 0F 10 45 ? 8B 55 14");

		// \x8B\x0D\x00\x00\x00\x00\x8B\x01\x8B\x50\x40\x68\x00\x00\x00\x00\xFF\xD2\x85\xC0\x74\x17 xx????xxxxxx????xxxxxxx
		Add("CvarPtr", "engine.dll", "Portal 2 Build 6879", "8B 0D ? ? ? ? 8B 01 8B 50 40 68 ? ? ? ? FF D2 85 C0 74 17", 2);
		
		// 
		// \x55\x8B\xEC\x56\x8B\x75\x08\x83\x3E\x02\x74\x11\x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x83\xC4\x04\x5E\x5D\xC3\x8B\x86\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00 xxxxxxxxxxxxx????xx????xxxxxxxx????xx????
		Add("g_pInputSystem", "engine.dll", "Portal 2 Build 6879", "55 8B EC 56 8B 75 08 83 3E 02 74 11 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 04 5E 5D C3 8B 86 ? ? ? ? 8B 0D ? ? ? ? ", 37);
		
		// 
		// \x55\x8B\xEC\x56\x8B\x75\x08\x83\xFE\xFF xxxxxxxxxx
		Add("Key_SetBinding", "engine.dll", "Portal 2 Build 6879", "55 8B EC 56 8B 75 08 83 FE FF");
		
		// 
		// \x55\x8B\xEC\x8B\x45\x0C\x53\x33\xDB\x56\x8B\xF1\x8B\x4D\x18\x80\x66\x20\xF9 xxxxxxxxxxxxxxxxxxx
		Add("ConCommand_Ctor1", "engine.dll", "Portal 2 Build 6879", "55 8B EC 8B 45 0C 53 33 DB 56 8B F1 8B 4D 18 80 66 20 F9");
		
		// 
		// \x55\x8B\xEC\x8B\x45\x0C\x53\x33\xDB\x56\x8B\xF1\x8B\x4D\x18\x80\x4E\x20\x02 xxxxxxxxxxxxxxxxxxx
		Add("ConCommand_Ctor2", "engine.dll", "Portal 2 Build 6879", "55 8B EC 8B 45 0C 53 33 DB 56 8B F1 8B 4D 18 80 4E 20 02");
		
		// CGameClient::SetSignonState
		// \x55\x8B\xEC\x56\x57\x8B\x7D\x08\x8B\xF1\x83\xFF\x02 xxxxxxxxxxxxx
		Add("SetSignonState", "engine.dll", "Portal 2 Build 6879", "55 8B EC 56 57 8B 7D 08 8B F1 83 FF 02");
		
		// CDemoUIPanel::OnFileSelected
		// \x55\x8B\xEC\x8B\x45\x08\x81\xEC\x00\x00\x00\x00\x56\x8B\xF1\x85\xC0\x0F\x84\x00\x00\x00\x00\x80\x38\x00\x0F\x84\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x8B\x11\x57\x68\x00\x00\x00\x00\x8D\xBD\x00\x00\x00\x00\x57\x50\x8B\x82\x00\x00\x00\x00\xFF\xD0\x6A\x0A xxxxxxxx????xxxxxxx????xxxxx????xx????xxxx????xx????xxxx????xxxx
		Add("OnFileSelected", "engine.dll", "Portal 2 Build 6879", "55 8B EC 8B 45 08 81 EC ? ? ? ? 56 8B F1 85 C0 0F 84 ? ? ? ? 80 38 00 0F 84 ? ? ? ? 8B 0D ? ? ? ? 8B 11 57 68 ? ? ? ? 8D BD ? ? ? ? 57 50 8B 82 ? ? ? ? FF D0 6A 0A");
		
		// CVEngineServer::GetGameDir
		// \x55\x8B\xEC\x8B\x45\x08\x85\xC0\x74\x12\x8B\x4D\x0C xxxxxxxxxxxxx
		Add("GetGameDir", "engine.dll", "Portal 2 Build 6879", "55 8B EC 8B 45 08 85 C0 74 12 8B 4D 0C");
		
		// 
		// 
		Add("engineClient", "engine.dll", "Portal 2 Build 6879", "55 8B EC 83 EC 10 B8 ? ? ? ? 99", 63);
		
		// CGameClient::SpawnPlayer
		// \x55\x8B\xEC\x83\xEC\x14\x80\x3D\x00\x00\x00\x00\x00\x56 xxxxxxxx?????x
		Add("m_bLoadgame", "engine.dll", "Portal 2 Build 6879", "55 8B EC 83 EC 14 80 3D ? ? ? ? ? 56", 8);
		
		// CGlobalVarsBase::curtime
		// \x89\x96\xC4\x00\x00\x00\x8B\x86\xC8\x00\x00\x00\x8B\xCE\xA3\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xD9\x1D\x00\x00\x00\x00\x8B\xCE\xE8\x00\x00\x00\x00\xD9\x1D xxxxxxxxxxxxxxx????x????xx????xxx????xx
		Add("curtime", "engine.dll", "Portal 2 Build 6879", "89 96 C4 00 00 00 8B 86 C8 00 00 00 8B CE A3 ? ? ? ? E8 ? ? ? ? D9 1D ? ? ? ? 8B CE E8 ? ? ? ? D9 1D", 26);

		// CBaseServer::m_szMapname[64]
		// \xD9\x00\x2C\xD9\xC9\xDF\xF1\xDD\xD8\x76\x00\x80\x00\x00\x00\x00\x00\x00 x?xxxxxxxx?x?????x
		Add("m_szMapname", "engine.dll", "Portal 2 Build 6879", "D9 ? 2C D9 C9 DF F1 DD D8 76 ? 80 ? ? ? ? ? 00", 13);

		// CBaseServer::m_szMapname[64]
		// \x76\x50\x80\x3D\x00\x00\x00\x00\x00\xB8\x00\x00\x00\x00 xxxx?????x????
		Add("m_szMapname", "engine.dll", "INFRA Build 6905", "76 50 80 3D ? ? ? ? ? B8 ? ? ? ? ", 10);

		// CDemoRecorder::CloseDemoFile
		// \x56\x8B\xF1\x57\x8D\x4E\x04\xE8\x00\x00\x00\x00\x84\xC0\x0F\x84\x00\x00\x00\x00\x80\xBE\x00\x00\x00\x00\x00 xxxxxxxx????xxxx????xx?????
		Add("CloseDemoFile", "engine.dll", "Portal 2 Build 6879", "56 8B F1 57 8D 4E 04 E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 80 BE ? ? ? ? ? ");

		// 
		// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x83\x3D\x00\x00\x00\x00\x00\x53 xxxxx????xx?????x
		Add("demorecorder", "engine.dll", "Portal 2 Build 6879", "55 8B EC 81 EC ? ? ? ? 83 3D ? ? ? ? ? 53", 43);

		// 
		// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x83\x3D\x00\x00\x00\x00\x00\x53 xxxxx????xx?????x
		Add("demoplayer", "engine.dll", "Portal 2 Build 6879", "55 8B EC 81 EC ? ? ? ? 83 3D ? ? ? ? ? 53", 79);

		// CL_PlayDemo_f
		// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x83\x3D\x00\x00\x00\x00\x00\x0F\x85\x00\x00\x00\x00\x8B\x45\x08\x83\x38\x02 xxxxx????xx?????xx????xxxxxx
		Add("PlayDemo", "engine.dll", "Portal 2 Build 6879", "55 8B EC 81 EC ? ? ? ? 83 3D ? ? ? ? ? 0F 85 ? ? ? ? 8B 45 08 83 38 02");

		// CDemoPlayer::StartPlayback
		// \x55\x8B\xEC\x53\x56\x57\x6A\x00\x8B\xF1 xxxxxxxxxx
		Add("StartPlayback", "engine.dll", "Portal 2 Build 6879", "55 8B EC 53 56 57 6A 00 8B F1");

		// CDemoRecorder::StopRecording
		// \x55\x8B\xEC\x51\x56\x8B\xF1\x8B\x06\x8B\x50\x10 xxxxxxxxxxxx
		Add("StopRecording", "engine.dll", "Portal 2 Build 6879", "55 8B EC 51 56 8B F1 8B 06 8B 50 10");

		// CDemoRecorder::StartupDemoFile
		// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x53\x8B\xD9\x80\xBB\x00\x00\x00\x00\x00\x56 xxxxx????xxxxx?????x
		Add("StartupDemoFile", "engine.dll", "Portal 2 Build 6879", "55 8B EC 81 EC ? ? ? ? 53 8B D9 80 BB ? ? ? ? ? 56");

		// CDemoPlayer::ReadPacket
		// \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x57\x8B\xF9\x8D\x4F\x04\x89\x7D\xB0 xxxxx????xxxxxxxxx
		Add("ReadPacket", "engine.dll", "Portal 2 Build 6879", "55 8B EC 81 EC ? ? ? ? 57 8B F9 8D 4F 04 89 7D B0", 414);

		// CON_COMMAND stop
		// \x83\x3D\x00\x00\x00\x00\x00\x75\x1F xx?????xx
		Add("Stop", "engine.dll", "Portal 2 Build 6879", "83 3D ? ? ? ? ? 75 1F");

		// ConVar_PrintDescription
		// \x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x83\xC4\x0C\x8B\xE5\x5D\xC3\x8D\x8D\x00\x00\x00\x00 x????xx????xxxxxxxxx????
		Add("PrintDescription", "engine.dll", "Portal 2 Build 6879", "68 ? ? ? ? FF 15 ? ? ? ? 83 C4 0C 8B E5 5D C3 8D 8D ? ? ? ? ", 1);

		// CBaseClientState::Disconnect
		// \x55\x8B\xEC\xDD\x05\x00\x00\x00\x00\x83\xEC\x18 xxxxx????xxx
		Add("Disconnect", "engine.dll", "Portal 2 Build 6879", "55 8B EC DD 05 ? ? ? ? 83 EC 18");

		// HostState_Frame
		// \x55\x8B\xEC\xD9\x45\x08\x51\xB9\x00\x00\x00\x00\xD9\x1C\x24\xE8\x00\x00\x00\x00\x5D\xC3 xxxxxxxx????xxxx????xx
		Add("HostStateFrame", "engine.dll", "Portal 2 Build 6879", "55 8B EC D9 45 08 51 B9 ? ? ? ? D9 1C 24 E8 ? ? ? ? 5D C3");

		// CHostState::m_currentState
		// \xC7\x05\x00\x00\x00\x00\x07\x00\x00\x00\xC3 xx????xxxxx
		Add("m_currentState", "engine.dll", "Portal 2 Build 6879", "C7 05 ? ? ? ? 07 00 00 00 C3", 2);

		// CBaseServer::SetPaused
		// \x55\x8B\xEC\x83\xEC\x14\x56\x8B\xF1\x83\x7E\x04\x03 xxxxxxxxxxxxx
		Add("SetPaused", "engine.dll", "Portal 2 Build 6879", "55 8B EC 83 EC 14 56 8B F1 83 7E 04 03");
	}
}