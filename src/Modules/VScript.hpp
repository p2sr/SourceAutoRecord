#pragma once

#include "Module.hpp"
#include "Utils.hpp"

class VScript : public Module {
public:
	// CVScriptGameSystem::CompileScript
	DECL_DETOUR_T(int, CompileScript, const char *scriptData, const char *scriptName);

	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE("vscript"); }
};

extern VScript *vscript;
