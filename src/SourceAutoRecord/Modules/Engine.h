#include <string>

using _SetSignonState = bool(__thiscall*)(void* thisptr, int state, int spawncount);
using _GetCommandBuffer = void*(__cdecl*)();
using _AddText = void(__cdecl*)(void* command_buffer, const char* text, int tickdelay);

enum SignonState {
	None = 0,
	Challenge = 1,
	Connected = 2,
	New = 3,
	Prespawn = 4,
	Spawn = 5,
	Full = 6,
	Changelevel = 7
};

// engine.dll
namespace Engine
{
	_SetSignonState Original_SetSignonState;
	_GetCommandBuffer GetCommandBuffer;
	_AddText AddText;

	bool __fastcall Detour_SetSignonState(void* thisptr, int edx, int state, int spawncount)
	{
		if (state == Prespawn) {
			//auto command = "record " + std::string(cfg.demName) + std::to_string(cfg.demCount);
			//if (cfg.saveRebinderEnabled) {
			//	command += ";bind " + std::string(cfg.saveKeyBinder) + " \"save " + std::string(cfg.saveNameBinder) + std::to_string(cfg.demCount);
			//	if (cfg.withSaveFlag) {
			//		command += ";echo #SAVE#\"";	// SourceRuns standard
			//	}
			//	else {
			//		command += "\"";
			//	}
			//}
			//if (cfg.reloadRebinderEnabled) {
			//	command += ";bind " + std::string(cfg.reloadKeyBinder) + " \"save " + std::string(cfg.reloadNameBinder) + std::to_string(cfg.demCount) + ";reload\"";
			//}
			//Original_Cbuf_AddText(Original_Cbuf_GetCommandBuffer(), command.c_str(), 0);
			//cfg.Update();
		}
		return Original_SetSignonState(thisptr, state, spawncount);
	}
}