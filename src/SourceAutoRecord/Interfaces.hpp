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

	void* Get(const char* filename, const char* interface)
	{
		auto handle = dlopen(filename, RTLD_NOLOAD | RTLD_NOW);
		if (!handle) {
			Console::DevWarning("Failed to open file %s!\n", filename);
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
			IGameMovement = Get("./portal2/bin/server.so", "GameMovement001");
			IVEngineClient = Get("./bin/engine.so", "VEngineClient015");
			IInputSystem = Get("./bin/inputsystem.so", "InputSystemVersion001");
			ISurface = Get("./bin/vguimatsurface.so", "VGUI_Surface031");
			IEngineVGui = Get("./bin/engine.so", "VEngineVGui001");
			IBaseClientDLL = Get("./portal2/bin/client.so", "VClient016");
			IEngineTool = Get("./bin/engine.so", "VENGINETOOL003");
			ISchemeManager = Get("./bin/vgui2.so", "VGUI_Scheme010");
		}
	}
}