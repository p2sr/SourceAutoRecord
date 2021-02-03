#include "Variable.hpp"

#include <cstring>

#include "Modules/Tier1.hpp"

#include "Game.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"

std::vector<Variable*>& Variable::GetList()
{
    static std::vector<Variable*> list;
    return list;
}

Variable::Variable()
    : ptr(nullptr)
    , originalFlags(0)
    , originalFnChangeCallback(nullptr)
    , version(SourceGame_Unknown)
    , isRegistered(false)
    , isReference(false)
    , hasCustomCallback(false)
    , isUnlocked(false)
{
}
Variable::~Variable()
{
    if (!this->isReference) {
        SAFE_DELETE(this->ptr)
    }
}
Variable::Variable(const char* name)
    : Variable()
{
    this->ptr = reinterpret_cast<ConVar*>(tier1->FindCommandBase(tier1->g_pCVar->ThisPtr(), name));
    this->isReference = true;
}
// Boolean or String
Variable::Variable(const char* name, const char* value, const char* helpstr, int flags, FnChangeCallback_t callback)
    : Variable()
{
    if (flags != 0)
        Create(name, value, flags, helpstr, true, 0, true, 1, callback);
    else
        Create(name, value, flags, helpstr, false, 0, false, 0, callback);
}
// Float
Variable::Variable(const char* name, const char* value, float min, const char* helpstr, int flags, FnChangeCallback_t callback)
    : Variable()
{
    Create(name, value, flags, helpstr, true, min, false, 0, callback);
}
// Float
Variable::Variable(const char* name, const char* value, float min, float max, const char* helpstr, int flags, FnChangeCallback_t callback)
    : Variable()
{
    Create(name, value, flags, helpstr, true, min, true, max, callback);
}
void Variable::Create(const char* name, const char* value, int flags, const char* helpstr, bool hasmin, float min, bool hasmax, float max, FnChangeCallback_t callback)
{
    this->ptr = new ConVar(name, value, flags, helpstr, hasmin, min, hasmax, max, callback);

    Variable::GetList().push_back(this);
}
void Variable::Realloc()
{
    if (sar.game->Is(SourceGame_Portal2Engine)) {
        auto newptr = new ConVar2(
            this->ptr->m_pszName,
            this->ptr->m_pszDefaultValue,
            this->ptr->m_nFlags,
            this->ptr->m_pszHelpString,
            this->ptr->m_bHasMin,
            this->ptr->m_fMinVal,
            this->ptr->m_bHasMax,
            this->ptr->m_fMaxVal);
        delete this->ptr;
        this->ptr = reinterpret_cast<ConVar*>(newptr);
    }
}
void Variable::AddCallBack(FnChangeCallback_t callback)
{
    if (sar.game->Is(SourceGame_Portal2Engine) && callback != nullptr) {
        this->ThisPtr2()->m_fnChangeCallback.Append(callback);
        this->hasCustomCallback = true;
        this->GetList().push_back(this);
    }
}
ConVar* Variable::ThisPtr()
{
    return this->ptr;
}
ConVar2* Variable::ThisPtr2()
{
    return reinterpret_cast<ConVar2*>(this->ptr);
}
bool Variable::GetBool()
{
    return !!GetInt();
}
int Variable::GetInt()
{
    return this->ptr->m_nValue;
}
float Variable::GetFloat()
{
    return this->ptr->m_fValue;
}
const char* Variable::GetString()
{
    return this->ptr->m_pszString;
}
const int Variable::GetFlags()
{
    return this->ptr->m_nFlags;
}
void Variable::SetValue(const char* value)
{
    Memory::VMT<_InternalSetValue>(this->ptr, Offsets::InternalSetValue)(this->ptr, value);
}
void Variable::SetValue(float value)
{
    Memory::VMT<_InternalSetFloatValue>(this->ptr, Offsets::InternalSetFloatValue)(this->ptr, value);
}
void Variable::SetValue(int value)
{
    Memory::VMT<_InternalSetIntValue>(this->ptr, Offsets::InternalSetIntValue)(this->ptr, value);
}
void Variable::SetFlags(int value)
{
    this->ptr->m_nFlags = value;
}
void Variable::AddFlag(int value)
{
    this->SetFlags(this->GetFlags() | value);
}
void Variable::RemoveFlag(int value)
{
    this->SetFlags(this->GetFlags() & ~(value));
}
void Variable::Unlock(bool asCheat)
{
    if (this->ptr) {
        this->originalFlags = this->ptr->m_nFlags;
        this->RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
        if (asCheat) {
            this->AddFlag(FCVAR_CHEAT);
        }

        if (this->isReference) {
            this->isUnlocked = true;
            this->GetList().push_back(this);
        }
    }
}
void Variable::Lock()
{
    if (this->ptr) {
        this->SetFlags(this->originalFlags);
        this->SetValue(this->ptr->m_pszDefaultValue);
        this->isUnlocked = false;
    }
}
void Variable::DisableChange()
{
    if (this->ptr) {
        if (sar.game->Is(SourceGame_Portal2Engine)) {
            this->originalSize = ((ConVar2*)this->ptr)->m_fnChangeCallback.m_Size;
            ((ConVar2*)this->ptr)->m_fnChangeCallback.m_Size = 0;
        } else if (sar.game->Is(SourceGame_HalfLife2Engine)) {
            this->originalFnChangeCallback = this->ptr->m_fnChangeCallback;
            this->ptr->m_fnChangeCallback = nullptr;
        }
    }
}
void Variable::EnableChange()
{
    if (this->ptr) {
        if (sar.game->Is(SourceGame_Portal2Engine)) {
            ((ConVar2*)this->ptr)->m_fnChangeCallback.m_Size = this->originalSize;
        } else if (sar.game->Is(SourceGame_HalfLife2Engine)) {
            this->ptr->m_fnChangeCallback = this->originalFnChangeCallback;
        }
    }
}
void Variable::UniqueFor(int version)
{
    this->version = version;
}
void Variable::Register()
{
    if (!this->isRegistered && !this->isReference && this->ptr) {
        this->isRegistered = true;
        FnChangeCallback_t callback = this->ptr->m_fnChangeCallback;
        this->Realloc();
        this->ptr->ConCommandBase_VTable = tier1->ConVar_VTable;
        this->ptr->ConVar_VTable = tier1->ConVar_VTable2;
        tier1->Create(this->ptr,
            this->ptr->m_pszName,
            this->ptr->m_pszDefaultValue,
            this->ptr->m_nFlags,
            this->ptr->m_pszHelpString,
            this->ptr->m_bHasMin,
            this->ptr->m_fMinVal,
            this->ptr->m_bHasMax,
            this->ptr->m_fMaxVal,
            callback);
    }
}
void Variable::Unregister()
{
    if (this->isRegistered && !this->isReference && this->ptr) {
        this->isRegistered = false;
        tier1->UnregisterConCommand(tier1->g_pCVar->ThisPtr(), this->ptr);
#ifdef _WIN32
        tier1->Dtor(this->ptr, 0);
#else
        tier1->Dtor(this->ptr);
#endif
        SAFE_DELETE(this->ptr)
    }
}
bool Variable::operator!()
{
    return this->ptr == nullptr;
}
void Variable::ClearAllCallbacks()
{
    for (auto var : Variable::GetList()) {
        if (var->hasCustomCallback) {
            var->ThisPtr2()->m_fnChangeCallback.Clear();
            var->hasCustomCallback = false;
        }
    }
}
int Variable::RegisterAll()
{
    auto result = 0;
    for (const auto& var : Variable::GetList()) {
        if (var->version != SourceGame_Unknown && !sar.game->Is(var->version)) {
            continue;
        }
        var->Register();
        ++result;
    }
    return result;
}
void Variable::UnregisterAll()
{
    for (const auto& var : Variable::GetList()) {
        var->Unregister();
    }
}
Variable* Variable::Find(const char* name)
{
    for (const auto& var : Variable::GetList()) {
        if (!std::strcmp(var->ThisPtr()->m_pszName, name)) {
            return var;
        }
    }
    return nullptr;
}
