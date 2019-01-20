#include "Variable.hpp"

#include <cstring>

#include "Modules/Tier1.hpp"

#include "Game.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"

Variable::Variable()
    : ptr(nullptr)
    , version(SourceGame_Unknown)
    , originalFlags(0)
    , originalfnChangeCallback(0)
    , isRegistered(false)
    , isReference(false)
{
}
Variable::~Variable()
{
    if (!isReference) {
        delete ptr;
        ptr = nullptr;
    }
}
Variable::Variable(const char* name)
    : Variable()
{
    this->ptr = reinterpret_cast<ConVar*>(tier1->FindCommandBase(tier1->g_pCVar->ThisPtr(), name));
    this->isReference = true;
}
// Boolean or String
Variable::Variable(const char* name, const char* value, const char* helpstr, int flags)
    : Variable()
{
    if (flags != 0)
        Create(name, value, flags, helpstr, true, 0, true, 1);
    else
        Create(name, value, flags, helpstr);
}
// Float
Variable::Variable(const char* name, const char* value, float min, const char* helpstr, int flags)
    : Variable()
{
    Create(name, value, flags, helpstr, true, min);
}
// Float
Variable::Variable(const char* name, const char* value, float min, float max, const char* helpstr, int flags)
    : Variable()
{
    Create(name, value, flags, helpstr, true, min, true, max);
}
void Variable::Create(const char* name, const char* value, int flags, const char* helpstr, bool hasmin, float min, bool hasmax,
    float max)
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
void Variable::PostInit()
{
    if (sar.game->version & SourceGame_Portal2Engine) {
        auto newptr = new ConVar2();
        newptr->m_pszName = this->ptr->m_pszName;
        newptr->m_pszHelpString = this->ptr->m_pszHelpString;
        newptr->m_nFlags = this->ptr->m_nFlags;
        newptr->m_pParent = newptr;
        newptr->m_pszDefaultValue = this->ptr->m_pszDefaultValue;
        newptr->m_StringLength = this->ptr->m_StringLength;
        newptr->m_pszString = new char[newptr->m_StringLength];
        std::memcpy(newptr->m_pszString, newptr->m_pszDefaultValue, newptr->m_StringLength);
        newptr->m_fValue = this->ptr->m_fValue;
        newptr->m_nValue = this->ptr->m_nValue;
        newptr->m_bHasMin = this->ptr->m_bHasMin;
        newptr->m_fMinVal = this->ptr->m_fMinVal;
        newptr->m_bHasMax = this->ptr->m_bHasMax;
        newptr->m_fMaxVal = this->ptr->m_fMaxVal;
        delete this->ptr;
        this->ptr = newptr;
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
            this->list.push_back(this);
        }
    }
}
void Variable::Lock()
{
    if (this->ptr) {
        this->ptr->m_nFlags = this->originalFlags;
    }
}
void Variable::DisableChange()
{
    if (this->ptr) {
        if (sar.game->version & SourceGame_Portal2Engine) {
            this->originalSize = ((ConVar2*)this->ptr)->m_Size;
            ((ConVar2*)this->ptr)->m_Size = 0;
        } else if (sar.game->version & SourceGame_HalfLife2Engine) {
            this->originalfnChangeCallback = this->ptr->m_fnChangeCallback;
            this->ptr->m_fnChangeCallback = nullptr;
        }
    }
}
void Variable::EnableChange()
{
    if (this->ptr) {
        if (sar.game->version & SourceGame_Portal2Engine) {
            ((ConVar2*)this->ptr)->m_Size = this->originalSize;
        } else if (sar.game->version & SourceGame_HalfLife2Engine) {
            this->ptr->m_fnChangeCallback = this->originalfnChangeCallback;
        }
    }
}
void Variable::UniqueFor(int version)
{
    this->version = version;
}
void Variable::Register()
{
    if (!this->isRegistered && !this->isReference) {
        this->isRegistered = true;
        this->PostInit();
        this->ptr->ConCommandBase_VTable = tier1->ConVar_VTable;
        this->ptr->ConVar_VTable = tier1->ConVar_VTable2;
        tier1->RegisterConCommand(tier1->g_pCVar->ThisPtr(), this->ptr);
        tier1->m_pConCommandList = this->ptr;
    }
}
void Variable::Unregister()
{
    if (this->isRegistered && !this->isReference) {
        this->isRegistered = false;
        tier1->UnregisterConCommand(tier1->g_pCVar->ThisPtr(), this->ptr);
    }
}
bool Variable::operator!()
{
    return this->ptr == nullptr;
}
int Variable::RegisterAll()
{
    auto result = 0;
    for (const auto& var : Variable::list) {
        if (var->version != SourceGame_Unknown && !(var->version & sar.game->version)) {
            continue;
        }
        var->Register();
        ++result;
    }
    return result;
}
void Variable::UnregisterAll()
{
    for (const auto& var : Variable::list) {
        var->Unregister();
    }
}
Variable* Variable::Find(const char* name)
{
    for (const auto& var : Variable::list) {
        if (!std::strcmp(var->ThisPtr()->m_pszName, name)) {
            return var;
        }
    }
    return nullptr;
}

std::vector<Variable*> Variable::list;
