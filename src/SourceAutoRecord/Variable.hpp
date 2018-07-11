#pragma once
#include "Modules/Tier1.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

using namespace Tier1;

struct Variable {
private:
    ConVar* ptr = nullptr;
    std::unique_ptr<uint8_t[]> data;
    bool isRegistered = false;
    int originalFlags = 0;

    using _ShouldRegisterCallback = bool(*)();
    _ShouldRegisterCallback shouldRegister = NULL;

    static std::vector<Variable*> list;

public:
    Variable() = default;
    Variable(const Variable& other) = delete;
    Variable(Variable&& other) = default;
    Variable& operator=(const Variable& other) = delete;
    Variable& operator=(Variable&& other) = default;

    Variable(const char* name)
    {
        this->ptr = reinterpret_cast<ConVar*>(FindCommandBase(g_pCVar->GetThisPtr(), name));
    }
    // Boolean or String
    Variable(const char* name, const char* value, const char* helpstr, int flags = FCVAR_NEVER_AS_STRING)
    {
        if (flags != 0)
            Create(name, value, flags, helpstr, true, 0, true, 1);
        else
            Create(name, value, flags, helpstr);
    }
    // Float
    Variable(const char* name, const char* value, float min, const char* helpstr, int flags = FCVAR_NEVER_AS_STRING)
    {
        Create(name, value, flags, helpstr, true, min);
    }
    // Float
    Variable(const char* name, const char* value, float min, float max, const char* helpstr, int flags = FCVAR_NEVER_AS_STRING)
    {
        Create(name, value, flags, helpstr, true, min, true, max);
    }
    void Create(const char* name, const char* value, int flags = 0, const char* helpstr = "", bool hasmin = false, float min = 0,
        bool hasmax = false, float max = 0)
    {
        auto size = sizeof(ConVar);
        data = std::make_unique<uint8_t[]>(size);
        this->ptr = reinterpret_cast<ConVar*>(data.get());
        std::memset(this->ptr, 0, size);

        // Store data temporarily so we can "register" it later in the engine
        this->ptr->m_pszName = name;
        this->ptr->m_pszDefaultValue = value;
        this->ptr->m_nFlags = flags;
        this->ptr->m_pszHelpString = helpstr;
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
        Memory::VMT<_InternalSetValue>(this->ptr, Offsets::InternalSetValue)(ptr, value);
    }
    void SetValue(float value)
    {
        Memory::VMT<_InternalSetFloatValue>(this->ptr, Offsets::InternalSetFloatValue)(ptr, value);
    }
    void SetValue(int value)
    {
        Memory::VMT<_InternalSetIntValue>(this->ptr, Offsets::InternalSetIntValue)(ptr, value);
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
    Variable* UniqueFor(_ShouldRegisterCallback callback)
    {
        shouldRegister = callback;
        return this;
    }
    void Register()
    {
        ConVarCtor(this->ptr,
            this->ptr->m_pszName,
            this->ptr->m_pszDefaultValue,
            this->ptr->m_nFlags,
            this->ptr->m_pszHelpString,
            this->ptr->m_bHasMin,
            this->ptr->m_fMinVal,
            this->ptr->m_bHasMax,
            this->ptr->m_fMaxVal);
        isRegistered = true;
    }
    void Unregister()
    {
        if (isRegistered)
            UnregisterConCommand(g_pCVar->GetThisPtr(), this->ptr);
    }
    static int RegisterAll()
    {
        auto result = 0;
        for (auto var : Variable::list) {
            if (var->shouldRegister && !var->shouldRegister()) {
                continue;
            }
            var->Register();
            result++;
        }
        return result;
    }
    static void UnregisterAll()
    {
        for (auto var : Variable::list) {
            var->Unregister();
        }
    }
    static Variable* Find(const char* name)
    {
        for (auto var : Variable::list) {
            if (std::strcmp(var->GetPtr()->m_pszName, name) == 0) {
                return var;
            }
        }
        return nullptr;
    }
};

std::vector<Variable*> Variable::list;