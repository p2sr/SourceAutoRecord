#pragma once
#include "vmthook/vmthook.h"

#include "Modules/Console.hpp"

#include "Game.hpp"
#include "SourceAutoRecord.hpp"
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
	void* ICVar;

	void* Get(const char* filename, const char* interface)
	{
		auto handle = Memory::GetModuleHandle(filename);
		if (!handle) {
			Console::DevWarning("Failed to open module %s!\n", filename);
			return nullptr;
		}

		auto factory = Memory::GetSymbolAddress(handle, "CreateInterface");
		if (!factory) {
			Console::DevWarning("Failed to find symbol CreateInterface for %s!\n", filename);
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
	void Log(const char* variable, const char* filename, const char* interfaceName)
	{
		auto path = Memory::GetModulePath(filename);
		if (!path) {
			Console::DevWarning("Failed to get module path for %s!\n", filename);
			return;
		}

		char command[1024];
		snprintf(command, sizeof(command), "strings \"%s\" | grep %s0", path, interfaceName);
		std::shared_ptr<FILE> pipe(popen(command, "r"), pclose);
		if (!pipe) {
			Console::DevWarning("popen error!\n");
			return;
		}

		std::array<char, 128> buffer;
		std::string result;
		while (!feof(pipe.get())) {
			if (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
				result += buffer.data();
			}
		}
		result.pop_back();

		Console::DevMsg("%s = Get(\"%s\", \"%s\");\n", variable, filename, result.c_str());
	}
	void Init()
	{
		if (Game::IsPortal2Engine()) {
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
			ICVar = Get("libvstdlib.so", "VEngineCvar007");
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
			ICVar = Get("libvstdlib.so", "VEngineCvar004");
		}
		else {
			Log("IVEngineClient", "engine.so", "VEngineClient");
			Log("IEngineVGui", "engine.so", "VEngineVGui");
			Log("IEngineTool", "engine.so", "VENGINETOOL");
			Log("IInputSystem", "inputsystem.so", "InputSystemVersion");
			Log("ISurface", "vguimatsurface.so", "VGUI_Surface");
			Log("ISchemeManager", "vgui2.so", "VGUI_Scheme");
			Log("IBaseClientDLL", "client.so", "VClient");
			Log("IClientEntityList", "client.so", "VClientEntityList");
			Log("IGameMovement", "server.so", "GameMovement");
			Log("IServerGameDLL", "server.so", "ServerGameDLL");
			Log("ICVar", "libvstdlib.so", "VEngineCvar");
		}
	}
}