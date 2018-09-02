#include "Tier1.hpp"

#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

bool Tier1::Init()
{
    this->g_pCVar = Interface::Create(this->Name(), "VEngineCvar0", false);
    if (this->g_pCVar) {
        this->RegisterConCommand = this->g_pCVar->Original<_RegisterConCommand>(Offsets::RegisterConCommand);
        this->UnregisterConCommand = this->g_pCVar->Original<_UnregisterConCommand>(Offsets::UnregisterConCommand);
        this->FindCommandBase = this->g_pCVar->Original<_FindCommandBase>(Offsets::FindCommandBase);

        this->m_pConCommandList = (ConCommandBase*)((uintptr_t)this->g_pCVar->ThisPtr() + Offsets::m_pConCommandList);

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

Tier1* tier1;

CBaseAutoCompleteFileList::CBaseAutoCompleteFileList(const char* cmdname, const char* subdir, const char* extension)
{
    m_pszCommandName = cmdname;
    m_pszSubDir = subdir;
    m_pszExtension = extension;
}
int CBaseAutoCompleteFileList::AutoCompletionFunc(char const* partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    return tier1->AutoCompletionFunc(this, partial, commands);
}
