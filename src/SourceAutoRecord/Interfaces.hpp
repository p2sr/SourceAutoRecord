#pragma once
#include "vmthook/vmthook.h"

#include "Modules/Console.hpp"

#include "Game.hpp"
#include "Utils.hpp"

namespace Interfaces
{
	void* IGameMovement;
	void* IVEngineClient;
	void* IInputSystem;
	void* ISurface;
	void* IEngineVGui;
	void* IBaseClientDLL;
	void* IEngineTool;
	void* ISchemeManager;
	void* IClientEntityList;
	void* IServerGameDLL;

	void* Get(const char* filename, const char* interface)
	{
		auto module = MODULEINFO();
		if (!GetModuleInformation(filename, &module)) {
			Console::DevWarning("Failed to get module info for %s!\n", filename);
			return nullptr;
		}

		auto handle = dlopen(module.modulePath, RTLD_NOLOAD | RTLD_NOW);
		if (!handle) {
			Console::DevWarning("Failed to open module %s!\n", filename);
			return nullptr;
		}

		auto factory = dlsym(handle, "CreateInterface");
		if (!factory) {
			Console::DevWarning("Failed to find symbol CreateInterface in %s!\n", filename);
			return nullptr;
		}

		typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);
		auto result = ((CreateInterfaceFn)(factory))(interface, nullptr);

		if (result) {
			Console::DevMsg("Found interface %s in %s!\n", interface, filename);
		}
		else {
			Console::DevWarning("Failed to find interface %s in %s!\n", interface, filename);
		}
		return result;
	}
	void Load()
	{
		if (Game::Version == Game::Portal2) {
			IVEngineClient = Get("engine.so", "VEngineClient015");
			IEngineVGui = Get("engine.so", "VEngineVGui001");
			IEngineTool = Get("engine.so", "VENGINETOOL003");
			IInputSystem = Get("inputsystem.so", "InputSystemVersion001");
			ISurface = Get("vguimatsurface.so", "VGUI_Surface031");
			ISchemeManager = Get("vgui2.so", "VGUI_Scheme010");
			IBaseClientDLL = Get("client.so", "VClient016");
			IClientEntityList = Get("client.so", "VClientEntityList003");
			IGameMovement = Get("server.so", "GameMovement001");
			IServerGameDLL = Get("server.so", "ServerGameDLL005");
		}
		else if (Game::Version == Game::Portal) {
			IVEngineClient = Get("engine.so", "VEngineClient013");
			IEngineVGui = Get("engine.so", "VEngineVGui001");
			IEngineTool = Get("engine.so", "VENGINETOOL003");
			IInputSystem = Get("inputsystem.so", "InputSystemVersion001");
			ISurface = Get("vguimatsurface.so", "VGUI_Surface030");
			ISchemeManager = Get("vgui2.so", "VGUI_Scheme010");
			IBaseClientDLL = Get("client.so", "VClient017");
			IClientEntityList = Get("client.so", "VClientEntityList003");
			IGameMovement = Get("server.so", "GameMovement001");
			IServerGameDLL = Get("server.so", "ServerGameDLL008");
		}
	}
}