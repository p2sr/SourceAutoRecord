#pragma once
#include "Interface.hpp"
#include "Module.hpp"
#include "Utils/SDK.hpp"

class Tier1 : public Module {
public:
    static Interface* g_pCVar;

    static _RegisterConCommand RegisterConCommand;
    static _UnregisterConCommand UnregisterConCommand;
    static _FindCommandBase FindCommandBase;

    static void* ConCommand_VTable;
    static void* ConVar_VTable;
    static void* ConVar_VTable2;
    static _AutoCompletionFunc AutoCompletionFunc;

    bool Init() override;
    void Shutdown() override;
};

extern Tier1* tier1;

struct CBaseAutoCompleteFileList {
    const char* m_pszCommandName;
    const char* m_pszSubDir;
    const char* m_pszExtension;

    CBaseAutoCompleteFileList(const char* cmdname, const char* subdir, const char* extension)
    {
        m_pszCommandName = cmdname;
        m_pszSubDir = subdir;
        m_pszExtension = extension;
    }
    int AutoCompletionFunc(char const* partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
    {
        return Tier1::AutoCompletionFunc(this, partial, commands);
    }
};
