#pragma once
#include "Interface.hpp"
#include "Module.hpp"
#include "Utils/SDK.hpp"

class Tier1 : public Module {
public:
    Interface* g_pCVar;
    ConCommandBase* m_pConCommandList;

    _RegisterConCommand RegisterConCommand;
    _UnregisterConCommand UnregisterConCommand;
    _FindCommandBase FindCommandBase;

    void* ConCommand_VTable;
    void* ConVar_VTable;
    void* ConVar_VTable2;
    _AutoCompletionFunc AutoCompletionFunc;

    bool Init() override;
    void Shutdown() override;
};

extern Tier1* tier1;

struct CBaseAutoCompleteFileList {
    const char* m_pszCommandName;
    const char* m_pszSubDir;
    const char* m_pszExtension;

    CBaseAutoCompleteFileList(const char* cmdname, const char* subdir, const char* extension);
    int AutoCompletionFunc(char const* partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
};
