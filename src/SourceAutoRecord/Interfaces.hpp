#pragma once
#include "vmthook/vmthook.h"

#include "Modules/Console.hpp"

#include "Game.hpp"
#include "Utils.hpp"

namespace Interfaces
{
	void* IGameMovement;
	void* IVEngineClient;
	void* IBaseClientDLL;
	void* IInputSystem;
	void* ISurface;
	void* IServerGameDLL;
	void* IEngineVGui;

	void* Get(const char* filename, const char* interface)
	{
		auto handle = dlopen(filename, RTLD_NOLOAD | RTLD_NOW);
		if (!handle) {
			Console::Warning("Failed to open file %s!\n", filename);
			return nullptr;
		}

		auto factory = dlsym(handle, "CreateInterface");
		if (!factory) {
			Console::Warning("Failed to find symbol CreateInterface in %s!\n", filename);
			return nullptr;
		}

		typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);
		auto result = ((CreateInterfaceFn)(factory))(interface, nullptr);

		if (result) {
			Console::PrintActive("Found interface %s in %s!\n", interface, filename);
		}
		else {
			Console::Warning("Failed to find interface % in %s!\n", interface, filename);
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
			IServerGameDLL = Get("./portal2/bin/server.so", "ServerGameDLL005");
			IEngineVGui = Get("./bin/engine.so", "VEngineVGui001");
		}
	}
}