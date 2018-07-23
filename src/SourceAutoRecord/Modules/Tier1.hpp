#pragma once
#include "Utils.hpp"

#define FCVAR_DEVELOPMENTONLY (1 << 1)
#define FCVAR_HIDDEN (1 << 4)
#define FCVAR_NEVER_AS_STRING (1 << 12)
#define FCVAR_CHEAT (1 << 14)

#define COMMAND_COMPLETION_MAXITEMS 64
#define COMMAND_COMPLETION_ITEM_LENGTH 64

namespace Tier1 {

VMT g_pCVar;

struct CCommand;
struct ConCommandBase;

using _CommandCallback = void (*)(const CCommand& args);
using _CommandCompletionCallback = int (*)(const char* partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
using _ConCommand = void(__func*)(void* thisptr, const char* name, void* callback, const char* helpstr, int flags, int* compfunc);
using _ConVar = void(__func*)(void* thisptr, const char* name, const char* value, int flags, const char* helpstr, bool hasmin, float min, bool hasmax, float max);
using _InternalSetValue = void(__func*)(void* thisptr, const char* value);
using _InternalSetFloatValue = void(__func*)(void* thisptr, float value);
using _InternalSetIntValue = void(__func*)(void* thisptr, int value);
using _UnregisterConCommand = void(__func*)(void* thisptr, ConCommandBase* pCommandBase);
using _FindCommandBase = void*(__func*)(void* thisptr, const char* name);
using _Command = void*(__func*)(void* thisptr, const char* name);
using _AutoCompletionFunc = int(__func*)(void* thisptr, char const* partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);

struct ConCommandBase {
    void* ConCommandBase_VTable; // 0
    ConCommandBase* m_pNext; // 4
    bool m_bRegistered; // 8
    const char* m_pszName; // 12
    const char* m_pszHelpString; // 16
    int m_nFlags; // 20
};

struct CCommand {
    enum {
        COMMAND_MAX_ARGC = 64,
        COMMAND_MAX_LENGTH = 512
    };
    int m_nArgc;
    int m_nArgv0Size;
    char m_pArgSBuffer[COMMAND_MAX_LENGTH];
    char m_pArgvBuffer[COMMAND_MAX_LENGTH];
    const char* m_ppArgv[COMMAND_MAX_ARGC];

    int ArgC() const
    {
        return this->m_nArgc;
    }
    const char* Arg(int nIndex) const
    {
        return this->m_ppArgv[nIndex];
    }
    const char* operator[](int nIndex) const
    {
        return Arg(nIndex);
    }
};

struct ICommandCallback {
    virtual void CommandCallback(const CCommand& command) = 0;
};

struct ConCommand : ConCommandBase {
    union {
        void* m_fnCommandCallbackV1;
        _CommandCallback m_fnCommandCallback;
        void* m_pCommandCallback;
    };

    union {
        _CommandCompletionCallback m_fnCompletionCallback;
        void* m_pCommandCompletionCallback;
    };

    bool m_bHasCompletionCallback : 1;
    bool m_bUsingNewCommandCallback : 1;
    bool m_bUsingCommandCallbackInterface : 1;
};

struct ConVar : ConCommandBase {
    void* ConVar_VTable; // 24
    ConVar* m_pParent; // 28
    const char* m_pszDefaultValue; // 32
    char* m_pszString; // 36
    int m_StringLength; // 40
    float m_fValue; // 44
    int m_nValue; // 48
    bool m_bHasMin; // 52
    float m_fMinVal; // 56
    bool m_bHasMax; // 60
    float m_fMaxVal; // 64
    // CUtlVector<FnChangeCallback_t> m_fnChangeCallback
    int m_fnChangeCallback; // 68
    int m_nAllocationCount; // 72
    int m_nGrowSize; // 76
    int m_Size; // 80
    int m_pElements; // 84
};

_ConCommand ConCommandCtor;
_ConVar ConVarCtor;
_UnregisterConCommand UnregisterConCommand;
_FindCommandBase FindCommandBase;

namespace Original {
    _AutoCompletionFunc AutoCompletionFunc;
}

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
        return Original::AutoCompletionFunc(this, partial, commands);
    }
};

bool Init()
{
    CREATE_VMT(Interfaces::ICVar, g_pCVar)
    {
        UnregisterConCommand = g_pCVar->GetOriginalFunction<_UnregisterConCommand>(Offsets::UnregisterConCommand);
        FindCommandBase = g_pCVar->GetOriginalFunction<_FindCommandBase>(Offsets::FindCommandBase);
    }

    auto cnc = SAR::Find("ConCommandCtor");
    if (cnc.Found) {
        ConCommandCtor = reinterpret_cast<_ConCommand>(cnc.Address);
    }

    auto cnv = SAR::Find("ConVarCtor");
    if (cnv.Found) {
        ConVarCtor = reinterpret_cast<_ConVar>(cnv.Address);
    }

    auto acf = SAR::Find("AutoCompletionFunc");
    if (acf.Found) {
        Original::AutoCompletionFunc = reinterpret_cast<_AutoCompletionFunc>(acf.Address);
    }

    return Interfaces::ICVar && cnc.Found && cnv.Found && acf.Found;
}
void Shutdown()
{
    DELETE_VMT(g_pCVar);
}
}
