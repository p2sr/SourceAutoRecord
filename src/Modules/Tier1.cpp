#include "Tier1.hpp"

#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

bool Tier1::Init()
{
#if _WIN32
    this->g_pCVar = Interface::Create(MODULE("vstdlib"), "VEngineCvar0", false);
#else
    this->g_pCVar = Interface::Create(MODULE("libvstdlib"), "VEngineCvar0", false);
#endif
    if (this->g_pCVar) {
        RegisterConCommand = this->g_pCVar->Original<_RegisterConCommand>(Offsets::RegisterConCommand);
        UnregisterConCommand = this->g_pCVar->Original<_UnregisterConCommand>(Offsets::UnregisterConCommand);
        FindCommandBase = this->g_pCVar->Original<_FindCommandBase>(Offsets::FindCommandBase);

        auto play = reinterpret_cast<ConCommand*>(FindCommandBase(this->g_pCVar->ThisPtr(), "play"));
        if (play) {
            this->ConCommand_VTable = play->ConCommandBase_VTable;
        }

        auto sv_lan = reinterpret_cast<ConVar*>(FindCommandBase(this->g_pCVar->ThisPtr(), "sv_lan"));
        if (sv_lan) {
            this->ConVar_VTable = sv_lan->ConCommandBase_VTable;
            this->ConVar_VTable2 = sv_lan->ConVar_VTable;
        }

        auto listdemo = reinterpret_cast<ConCommand*>(FindCommandBase(this->g_pCVar->ThisPtr(), "listdemo"));
        if (listdemo && listdemo->m_fnCompletionCallback) {
            auto callback = (uintptr_t)listdemo->m_fnCompletionCallback + Offsets::AutoCompletionFunc;
            this->AutoCompletionFunc = Memory::Read<_AutoCompletionFunc>(callback);
        }
    }

    return this->hasLoaded = this->g_pCVar
        && this->ConCommand_VTable
        && this->ConVar_VTable
        && this->ConVar_VTable2
        && this->AutoCompletionFunc;
}
void Tier1::Shutdown()
{
    Interface::Delete(this->g_pCVar);
}

Interface* Tier1::g_pCVar;
_RegisterConCommand Tier1::RegisterConCommand;
_UnregisterConCommand Tier1::UnregisterConCommand;
_FindCommandBase Tier1::FindCommandBase;
void* Tier1::ConCommand_VTable;
void* Tier1::ConVar_VTable;
void* Tier1::ConVar_VTable2;
_AutoCompletionFunc Tier1::AutoCompletionFunc;

Tier1* tier1;
