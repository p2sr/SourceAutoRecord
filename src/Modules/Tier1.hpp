#pragma once
#include "Interface.hpp"
#include "Module.hpp"
#include "Utils.hpp"

#ifdef _WIN32
#define TIER1 "vstdlib"
#else
#define TIER1 "libvstdlib"
#endif

class Tier1 : public Module {
public:
    Interface* g_pCVar = nullptr;

    _RegisterConCommand RegisterConCommand = nullptr;
    _UnregisterConCommand UnregisterConCommand = nullptr;
    _FindCommandBase FindCommandBase = nullptr;

    ConCommandBase* m_pConCommandList = nullptr;

    void* ConCommand_VTable = nullptr;
    void* ConVar_VTable = nullptr;
    void* ConVar_VTable2 = nullptr;
    _AutoCompletionFunc AutoCompletionFunc = nullptr;

public:
    bool Init() override;
    void Shutdown() override;
    const char* Name() override { return MODULE(TIER1); }
};

extern Tier1* tier1;

struct CBaseAutoCompleteFileList {
    const char* m_pszCommandName;
    const char* m_pszSubDir;
    const char* m_pszExtension;

    CBaseAutoCompleteFileList(const char* cmdname, const char* subdir, const char* extension);
    int AutoCompletionFunc(char const* partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
};
