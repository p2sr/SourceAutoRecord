#pragma once
#include "Modules/Tier1.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

using namespace Tier1;

struct Variable {
private:
    ConVar* ptr;
    bool isRegistered;
    bool isReference;
    int originalFlags;

    using _ShouldRegisterCallback = bool (*)();
    _ShouldRegisterCallback shouldRegister;

    static std::vector<Variable*> list;

public:
    Variable()
        : ptr(nullptr)
        , isRegistered(false)
        , isReference(nullptr)
        , originalFlags(0)
        , shouldRegister(nullptr)
    {
    }
    ~Variable()
    {
        if (!isReference) {
            delete ptr;
        }
    }
    Variable(const char* name)
        : Variable()
    {
        this->ptr = reinterpret_cast<ConVar*>(FindCommandBase(g_pCVar->GetThisPtr(), name));
        this->isReference = true;
    }
    // Boolean or String
    Variable(const char* name, const char* value, const char* helpstr, int flags = FCVAR_NEVER_AS_STRING)
        : Variable()
    {
        if (flags != 0)
            Create(name, value, flags, helpstr, true, 0, true, 1);
        else
            Create(name, value, flags, helpstr);
    }
    // Float
    Variable(const char* name, const char* value, float min, const char* helpstr, int flags = FCVAR_NEVER_AS_STRING)
        : Variable()
    {
        Create(name, value, flags, helpstr, true, min);
    }
    // Float
    Variable(const char* name, const char* value, float min, float max, const char* helpstr, int flags = FCVAR_NEVER_AS_STRING)
        : Variable()
    {
        Create(name, value, flags, helpstr, true, min, true, max);
    }
    void Create(const char* name, const char* value, int flags = 0, const char* helpstr = "", bool hasmin = false, float min = 0,
        bool hasmax = false, float max = 0)
    {
        this->ptr = new ConVar();
        this->ptr->m_pszName = name;
        this->ptr->m_pszHelpString = helpstr;
        this->ptr->m_nFlags = flags;
        this->ptr->m_pParent = this->ptr;
        this->ptr->m_pszDefaultValue = value;
        this->ptr->m_StringLength = std::strlen(this->ptr->m_pszDefaultValue) + 1;
        this->ptr->m_pszString = new char[this->ptr->m_StringLength];
        std::memcpy(this->ptr->m_pszString, this->ptr->m_pszDefaultValue, this->ptr->m_StringLength);
        this->ptr->m_fValue = (float)std::atof(this->ptr->m_pszString);
        this->ptr->m_nValue = (int)this->ptr->m_fValue;
        this->ptr->m_bHasMin = hasmin;
        this->ptr->m_fMinVal = min;
        this->ptr->m_bHasMax = hasmax;
        this->ptr->m_fMaxVal = max;

        Variable::list.push_back(this);
    }
    ConVar* GetPtr()
    {
        return this->ptr;
    }
    bool GetBool()
    {
        return !!GetInt();
    }
    int GetInt()
    {
        return this->ptr->m_nValue;
    }
    float GetFloat()
    {
        return this->ptr->m_fValue;
    }
    const char* GetString()
    {
        return this->ptr->m_pszString;
    }
    const int GetFlags()
    {
        return this->ptr->m_nFlags;
    }
    void SetValue(const char* value)
    {
        Memory::VMT<_InternalSetValue>(this->ptr, Offsets::InternalSetValue)(this->ptr, value);
    }
    void SetValue(float value)
    {
        Memory::VMT<_InternalSetFloatValue>(this->ptr, Offsets::InternalSetFloatValue)(this->ptr, value);
    }
    void SetValue(int value)
    {
        Memory::VMT<_InternalSetIntValue>(this->ptr, Offsets::InternalSetIntValue)(this->ptr, value);
    }
    void SetFlags(int value)
    {
        this->ptr->m_nFlags = value;
    }
    void AddFlag(int value)
    {
        SetFlags(GetFlags() | value);
    }
    void RemoveFlag(int value)
    {
        SetFlags(GetFlags() & ~(value));
    }
    void Unlock(bool asCheat = true)
    {
        if (this->ptr) {
            this->originalFlags = this->ptr->m_nFlags;
            this->RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
            if (asCheat) {
                this->AddFlag(FCVAR_CHEAT);
            }
        }
    }
    void Lock()
    {
        if (this->ptr) {
            this->ptr->m_nFlags = this->originalFlags;
        }
    }
    void UniqueFor(_ShouldRegisterCallback callback)
    {
        this->shouldRegister = callback;
    }
    void Register()
    {
        if (!this->isRegistered) {
            this->ptr->ConCommandBase_VTable = Original::ConVar_VTable;
            this->ptr->ConVar_VTable = Original::ConVar_VTable2;
            RegisterConCommand(g_pCVar->GetThisPtr(), this->ptr);
        }
        this->isRegistered = true;
    }
    void Unregister()
    {
        if (this->isRegistered) {
            UnregisterConCommand(g_pCVar->GetThisPtr(), this->ptr);
        }
        this->isRegistered = false;
    }
    static int RegisterAll()
    {
        auto result = 0;
        for (const auto& var : Variable::list) {
            if (var->shouldRegister && !var->shouldRegister()) {
                continue;
            }
            var->Register();
            ++result;
        }
        return result;
    }
    static void UnregisterAll()
    {
        for (const auto& var : Variable::list) {
            var->Unregister();
        }
    }
    static Variable* Find(const char* name)
    {
        for (const auto& var : Variable::list) {
            if (!std::strcmp(var->GetPtr()->m_pszName, name)) {
                return var;
            }
        }
        return nullptr;
    }
};

std::vector<Variable*> Variable::list;
