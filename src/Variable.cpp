#include "Variable.hpp"

#include "Modules/Tier1.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

Variable::Variable()
    : ptr(nullptr)
    , originalFlags(0)
    , originalfnChangeCallback(0)
    , shouldRegister(nullptr)
    , isRegistered(false)
    , isReference(nullptr)
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
    this->ptr = reinterpret_cast<ConVar*>(Tier1::FindCommandBase(Tier1::g_pCVar->ThisPtr(), name));
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
ConVar* Variable::ThisPtr()
{
    return this->ptr;
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
    SetFlags(GetFlags() | value);
}
void Variable::RemoveFlag(int value)
{
    SetFlags(GetFlags() & ~(value));
}
void Variable::Unlock(bool asCheat)
{
    if (this->ptr) {
        this->originalFlags = this->ptr->m_nFlags;
        this->RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
        if (asCheat) {
            this->AddFlag(FCVAR_CHEAT);
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
#ifdef HL2_OPTIMISATION
        this->originalfnChangeCallback = this->ptr->originalfnChangeCallback;
        this->originalfnChangeCallback = nullptr;
#else
        if (game->IsHalfLife2Engine()) {
            this->originalfnChangeCallback = this->ptr->m_pMemory;
            this->ptr->m_pMemory = nullptr;
        } else {
            this->originalSize = this->ptr->m_Size;
            this->ptr->m_Size = 0;
        }
#endif
    }
}
void Variable::EnableChange()
{
    if (this->ptr) {
#ifdef HL2_OPTIMISATION
        this->ptr->originalfnChangeCallback = this->originalfnChangeCallback;
#else
        if (game->IsHalfLife2Engine()) {
            this->ptr->m_pMemory = this->originalfnChangeCallback;
        } else {
            this->ptr->m_Size = this->originalSize;
        }
#endif
    }
}
void Variable::UniqueFor(_ShouldRegisterCallback callback)
{
    this->shouldRegister = callback;
}
void Variable::Register()
{
    if (!this->isRegistered) {
        this->ptr->ConCommandBase_VTable = Tier1::ConVar_VTable;
        this->ptr->ConVar_VTable = Tier1::ConVar_VTable2;
        Tier1::RegisterConCommand(Tier1::g_pCVar->ThisPtr(), this->ptr);
    }
    this->isRegistered = true;
}
void Variable::Unregister()
{
    if (this->isRegistered) {
        Tier1::UnregisterConCommand(Tier1::g_pCVar->ThisPtr(), this->ptr);
    }
    this->isRegistered = false;
}
bool Variable::operator!()
{
    return this->ptr == nullptr;
}
int Variable::RegisterAll()
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
