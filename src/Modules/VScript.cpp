#include "VScript.hpp"

#include "Checksum.hpp"
#include "Game.hpp"
#include "Hook.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"

REDECL(VScript::CompileScript);

extern Hook g_VScriptCompileScriptHook;
DETOUR_T(int, VScript::CompileScript, const char *scriptData, const char *scriptName) {
	if (scriptName && *scriptName) {
		RecordRuntimeVscriptChecksum(scriptName, scriptData);
	}

	g_VScriptCompileScriptHook.Disable();
	auto ret = VScript::CompileScript(thisptr, scriptData, scriptName);
	g_VScriptCompileScriptHook.Enable();
	return ret;
}
Hook g_VScriptCompileScriptHook(&VScript::CompileScript_Hook);

bool VScript::Init() {
	VScript::CompileScript = (VScript::_CompileScript)Memory::Scan(this->Name(), Offsets::VScript_CompileScript);
	if (!VScript::CompileScript) {
		console->Warning("[sar] failed to find VScript_CompileScript\n");
		return false;
	}

	g_VScriptCompileScriptHook.SetFunc(VScript::CompileScript);
	return this->hasLoaded = true;
}

void VScript::Shutdown() {
}

VScript *vscript;
