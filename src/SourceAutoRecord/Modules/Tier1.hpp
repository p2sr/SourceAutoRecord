#pragma once
#include "vmthook/vmthook.h"

#define __cdecl __attribute__((__cdecl__))

#define FCVAR_DEVELOPMENTONLY (1 << 1)
#define FCVAR_HIDDEN (1 << 4)
#define FCVAR_NEVER_AS_STRING (1 << 12)
#define FCVAR_CHEAT (1 << 14)

#define COMMAND_COMPLETION_MAXITEMS 64
#define COMMAND_COMPLETION_ITEM_LENGTH 64

namespace Tier1 {

struct CCommand;

using _CommandCallback = void (*)();
using _CommandCallbackArgs = void (*)(const CCommand& args);
using _CommandCallbackCompletion = int (*)(const char* partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
using _ConCommand = void(__cdecl*)(void* thisptr, const char* name, void* callback, const char* helpstr, int flags, int* compfunc);
using _ConVar = void(__cdecl*)(void* thisptr, const char* name, const char* value, int flags, const char* helpstr, bool hasmin, float min, bool hasmax, float max);
using _InternalSetValue = void(__cdecl*)(void* thisptr, const char* value);
using _InternalSetFloatValue = void(__cdecl*)(void* thisptr, float value);
using _InternalSetIntValue = void(__cdecl*)(void* thisptr, int value);
using _FindVar = void*(__cdecl*)(void* thisptr, const char* name);
using _AutoCompletionFunc = int(__cdecl*)(void* thisptr, char const* partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);

std::unique_ptr<VMTHook> g_pCVar;

namespace Original {
    _ConCommand ConCommandCtor;
    _ConCommand ConCommandCtor2;
    _ConVar ConVarCtor;
    _FindVar FindVar;
    _AutoCompletionFunc AutoCompletionFunc;
}

int ConCommandCount = 0;
int ConVarCount = 0;

struct ConCommandBase {
    void* VMT;
    ConCommandBase* m_pNext;
    bool m_bRegistered;
    const char* m_pszName;
    const char* m_pszHelpString;
    int flags;
};

struct CCommand {
    enum {
        COMMAND_MAX_ARGC = 64,
        COMMAND_MAX_LENGTH = 512,
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

struct ConCommand : ConCommandBase {
    union {
        void* m_fnCommandCallbackV1;
        void* m_fnCommandCallback;
        void* m_pCommandCallback;
    };

    union {
        void* m_fnCompletionCallback;
        void* m_pCommandCompletionCallback;
    };

    bool m_bHasCompletionCallback : 1;
    bool m_bUsingNewCommandCallback : 1;
    bool m_bUsingCommandCallbackInterface : 1;
};

struct ConVar : ConCommandBase {
    void* VMT;
    ConVar* m_pParent;
    const char* m_pszDefaultValue;
    char* m_pszString;
    int m_StringLength;
    float m_fValue;
    int m_nValue;
    bool m_bHasMin;
    float m_fMinVal;
    bool m_bHasMax;
    float m_fMaxVal;
    void* m_fnChangeCallback;
    int unk1;
    int unk2;
    int unk3;
    int unk4;
};

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
    auto cnc = SAR::Find("ConCommand_Ctor1");
    auto cnc2 = SAR::Find("ConCommand_Ctor2");
    if (cnc.Found && cnc2.Found) {
        Original::ConCommandCtor = reinterpret_cast<_ConCommand>(cnc.Address);
        Original::ConCommandCtor2 = reinterpret_cast<_ConCommand>(cnc2.Address);
    }

    auto cnv = SAR::Find("ConVar_Ctor3");
    if (cnv.Found) {
        Original::ConVarCtor = reinterpret_cast<_ConVar>(cnv.Address);
    }

    if (Interfaces::ICVar) {
        g_pCVar = std::make_unique<VMTHook>(Interfaces::ICVar);
        Original::FindVar = g_pCVar->GetOriginalFunction<_FindVar>(Offsets::FindVar);
    }

    auto acf = SAR::Find("AutoCompletionFunc");
    if (acf.Found) {
        Original::AutoCompletionFunc = reinterpret_cast<_AutoCompletionFunc>(acf.Address);
    }

    return cnc.Found && cnc2.Found && cnv.Found && Interfaces::ICVar && acf.Found;
}
}