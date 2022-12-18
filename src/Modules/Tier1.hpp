#pragma once
#include "Interface.hpp"
#include "Module.hpp"
#include "Utils.hpp"

#ifdef _WIN32
#	define TIER1 "vstdlib"
#else
#	define TIER1 "libvstdlib"
#endif

class Tier1 : public Module {
public:
	Interface *g_pCVar = nullptr;

	_RegisterConCommand RegisterConCommand = nullptr;
	_UnregisterConCommand UnregisterConCommand = nullptr;
	_FindCommandBase FindCommandBase = nullptr;
	_InstallGlobalChangeCallback InstallGlobalChangeCallback = nullptr;
	_RemoveGlobalChangeCallback RemoveGlobalChangeCallback = nullptr;

	ConCommandBase *m_pConCommandList = nullptr;
	CUtlVector<IConsoleDisplayFunc *> *m_DisplayFuncs = nullptr;
	IConsoleDisplayFunc *orig_display_func = nullptr;

	void *ConCommand_VTable = nullptr;
	void *ConVar_VTable = nullptr;
	void *ConVar_VTable2 = nullptr;

#ifdef _WIN32
	using _Dtor = int(__rescall *)(ConVar *thisptr, char a2);
#else
	using _Dtor = int(__rescall *)(ConVar *thisptr);
#endif
	using _Create = int(__rescall *)(ConVar *thisptr, const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, bool bMin, float fMin, bool bMax, float fMax, FnChangeCallback_t callback);
	using _KeyValuesSystem = IKeyValuesSystem *(__cdecl *)();

	_Dtor Dtor = nullptr;
	_Create Create = nullptr;
	_KeyValuesSystem KeyValuesSystem = nullptr;

public:
	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE(TIER1); }
};

extern Tier1 *tier1;
