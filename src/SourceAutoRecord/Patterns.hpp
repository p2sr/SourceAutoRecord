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
		//Create("client.so", "GetLocalPlayer");
		Create("engine.so", "CvarPtr"); // ?
		Create("engine.so", "ConVar_Ctor3");
		Create("engine.so", "ConCommand_Ctor1");
		Create("engine.so", "ConCommand_Ctor2");
		Create("engine.so", "m_bLoadgame"); // TODO?
		Create("engine.so", "m_szMapname"); // TODO?
		Create("engine.so", "demorecorder"); // TODO?
		Create("engine.so", "Key_SetBinding");
		Create("engine.so", "demoplayer"); // TODO?
		Create("engine.so", "ClientState"); // ?
		Create("vguimatsurface.so", "StartDrawing");
		Create("vguimatsurface.so", "FinishDrawing");

		if (Game::Version == Game::Portal2){
			Portal2::Load();
		}
	}

	namespace Portal2
	{
		void Load()
		{
			// client.so


			// \x55\x89\xE5\x83\xEC\x00\x8B\x45\x00\x83\xF8\x00\x74\x00\xC9 xxxxx?xx?xx?x?x
			/* Add("GetLocalPlayer", "Portal 2 Build 7054",
				"C_BasePlayer::GetLocalPlayer",
				"55 89 E5 83 EC ? 8B 45 ? 83 F8 ? 74 ? C9"); */


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

			// \x55\x89\xE5\x83\xEC\x00\x89\x5D\x00\x89\xC3\xA1\x00\x00\x00\x00\x89\x75\x00\x89\xD6\x85\xC0 xxxxx?xx?xxx????xx?xxxx
			Add("demorecorder", "Portal 2 Build 7054",
				"demorecorder",
				"55 89 E5 83 EC ? 89 5D ? 89 C3 A1 ? ? ? ? 89 75 ? 89 D6 85 C0",
				26);

			// \x55\x89\xE5\x83\xEC\x00\x8B\x45\x00\x89\x75\x00\x89\x5D\x00\x8B\x75\x00\x89\x7D\x00\x83\xF8\x00\x0F\x84\x00\x00\x00\x00 xxxxx?xx?xx?xx?xx?xx?xx?xx????
			Add("Key_SetBinding", "Portal 2 Build 7054",
				"Key_SetBinding",
				"55 89 E5 83 EC ? 8B 45 ? 89 75 ? 89 5D ? 8B 75 ? 89 7D ? 83 F8 ? 0F 84 ? ? ? ? ");

			// \x55\x89\xE5\x83\xEC\x00\x89\x5D\x00\x89\xC3\xA1\x00\x00\x00\x00\x89\x75\x00\x89\xD6\x85\xC0 xxxxx?xx?xxx????xx?xxxx
			Add("demoplayer", "Portal 2 Build 7054",
				"demoplayer",
				"55 89 E5 83 EC ? 89 5D ? 89 C3 A1 ? ? ? ? 89 75 ? 89 D6 85 C0",
				43);

			// \x55\x89\xE5\x83\xEC\x00\xC7\x44\x24\x00\x00\x00\x00\x00\xC7\x04\x24\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xC9\xC3\x8D\x74\x26\x00\x55\x89\xE5\x57\x56\x53\x83\xEC\x00\xA1\x00\x00\x00\x00 xxxxx?xxx?????xxx????x????xxxxxxxxxxxxxx?x????
			Add("ClientState", "Portal 2 Build 7054",
				"CClientState::cl",
				"55 89 E5 83 EC ? C7 44 24 ? ? ? ? ? C7 04 24 ? ? ? ? E8 ? ? ? ? C9 C3 8D 74 26 00 55 89 E5 57 56 53 83 EC ? A1 ? ? ? ? ");


			// vguimatsurface.so


			// \x55\x89\xE5\x53\x83\xEC\x00\x80\x3D\x00\x00\x00\x00\x00\x8B\x5D\x00\x0F\x84\x00\x00\x00\x00 xxxxxx?xx?????xx?xx????
			Add("StartDrawing", "Portal 2 Build 7054",
				"CMatSystemSurface::StartDrawing",
				"55 89 E5 53 83 EC ? 80 3D ? ? ? ? ? 8B 5D ? 0F 84 ? ? ? ? ");

			// \x55\x89\xE5\x53\x83\xEC\x00\xC7\x04\x24\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xA1\x00\x00\x00\x00 xxxxxx?xxx????x????x????
			Add("FinishDrawing", "Portal 2 Build 7054",
				"CMatSystemSurface::FinishDrawing",
				"55 89 E5 53 83 EC ? C7 04 24 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? ");
		}
	}
}