#pragma once

#include "Utils/Platform.hpp"
#include "UtlMemory.hpp"
#include "Color.hpp"

#define FCVAR_DEVELOPMENTONLY (1 << 1)
#define FCVAR_HIDDEN (1 << 4)
#define FCVAR_NEVER_AS_STRING (1 << 12)
#define FCVAR_CHEAT (1 << 14)
#define FCVAR_DONTRECORD (1 << 17)

#define COMMAND_COMPLETION_MAXITEMS 64
#define COMMAND_COMPLETION_ITEM_LENGTH 64

struct CCommand;
struct ConCommandBase;

typedef void (*FnChangeCallback_t)(void *var, const char *pOldValue, float flOldValue);

using _CommandCallback = void (*)(const CCommand &args);
using _CommandCompletionCallback = int (*)(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
using _InternalSetValue = void(__rescalll *)(void *thisptr, const char *value);
using _InternalSetFloatValue = void(__rescalll *)(void *thisptr, float value);
using _InternalSetIntValue = void(__rescalll *)(void *thisptr, int value);
using _RegisterConCommand = void(__rescalll *)(void *thisptr, ConCommandBase *pCommandBase);
using _UnregisterConCommand = void(__rescalll *)(void *thisptr, ConCommandBase *pCommandBase);
using _FindCommandBase = void *(__rescalll *)(void *thisptr, const char *name);
using _InstallGlobalChangeCallback = void(__rescalll *)(void *thisptr, FnChangeCallback_t callback);
using _RemoveGlobalChangeCallback = void(__rescalll *)(void *thisptr, FnChangeCallback_t callback);

class IConVar {
public:
	virtual void SetValue(const char *pValue) = 0;
	virtual void SetValue(float flValue) = 0;
	virtual void SetValue(int nValue) = 0;
	virtual void SetValue(Color value) = 0;
	virtual const char *GetName(void) const = 0;
	virtual const char *GetBaseName(void) const = 0;
	virtual bool IsFlagSet(int nFlag) const = 0;
	virtual int GetSplitScreenPlayerSlot() const = 0;
};

struct ConCommandBase {
	ConCommandBase(const char *name, int flags, const char *helpstr)
		: m_pNext(nullptr)
		, m_bRegistered(false)
		, m_pszName(name)
		, m_pszHelpString(helpstr)
		, m_nFlags(flags) {
	}

	// if we actually put a virtual destructor here, EVERYTHING BREAKS
	// so put dummy methods instead
	virtual void _dtor(){};
#ifndef _WIN32
	virtual void _dtor1(){};
#endif
	virtual bool IsCommand() const { return false; };  // will get overwritten anyway lol
	// Note: vtable incomplete, but sufficient

	ConCommandBase *m_pNext;      // 4
	bool m_bRegistered;           // 8
	const char *m_pszName;        // 12
	const char *m_pszHelpString;  // 16
	int m_nFlags;                 // 20
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
	const char *m_ppArgv[COMMAND_MAX_ARGC];

	int ArgC() const {
		return this->m_nArgc;
	}
	const char *Arg(int nIndex) const {
		return this->m_ppArgv[nIndex];
	}
	const char *operator[](int nIndex) const {
		return Arg(nIndex);
	}
};

struct ConCommand : ConCommandBase {
	union {
		void *m_fnCommandCallbackV1;
		_CommandCallback m_fnCommandCallback;
		void *m_pCommandCallback;
	};

	union {
		_CommandCompletionCallback m_fnCompletionCallback;
		void *m_pCommandCompletionCallback;
	};

	bool m_bHasCompletionCallback : 1;
	bool m_bUsingNewCommandCallback : 1;
	bool m_bUsingCommandCallbackInterface : 1;

	ConCommand(const char *pName, _CommandCallback callback, const char *pHelpString, int flags, _CommandCompletionCallback completionFunc)
		: ConCommandBase(pName, flags, pHelpString)
		, m_fnCommandCallback(callback)
		, m_fnCompletionCallback(completionFunc)
		, m_bHasCompletionCallback(completionFunc != nullptr)
		, m_bUsingNewCommandCallback(true)
		, m_bUsingCommandCallbackInterface(false) {
	}
};

struct ConVar : ConCommandBase {
	void *ConVar_VTable;                                // 24
	ConVar *m_pParent;                                  // 28
	const char *m_pszDefaultValue;                      // 32
	char *m_pszString;                                  // 36
	int m_StringLength;                                 // 40
	float m_fValue;                                     // 44
	int m_nValue;                                       // 48
	bool m_bHasMin;                                     // 52
	float m_fMinVal;                                    // 56
	bool m_bHasMax;                                     // 60
	float m_fMaxVal;                                    // 64
	CUtlVector<FnChangeCallback_t> m_fnChangeCallback;  // 68

	ConVar(const char *name, const char *value, int flags, const char *helpstr, bool hasmin, float min, bool hasmax, float max)
		: ConCommandBase(name, flags, helpstr)
		, ConVar_VTable(nullptr)
		, m_pParent(nullptr)
		, m_pszDefaultValue(value)
		, m_pszString(nullptr)
		, m_StringLength(0)
		, m_fValue(0.0f)
		, m_nValue(0)
		, m_bHasMin(hasmin)
		, m_fMinVal(min)
		, m_bHasMax(hasmax)
		, m_fMaxVal(max)
		, m_fnChangeCallback() {
	}
};



class IConsoleDisplayFunc {
public:
	virtual void ColorPrint(const Color &clr, const char *msg) = 0;
	virtual void Print(const char *msg) = 0;
	virtual void DPrint(const char *msg) = 0;
	virtual void GetConsoleText(char *text, size_t bufSize) const = 0;
};
