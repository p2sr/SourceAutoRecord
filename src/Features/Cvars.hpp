#pragma once
#include "Modules/Console.hpp"
#include "Modules/Tier1.hpp"

#include "Feature.hpp"

#include "Command.hpp"
#include "Offsets.hpp"
#include "Variable.hpp"

#include "Utils/Memory.hpp"

class Cvars : public Feature {
public:
    Cvars();
    void ListAll();
    int Dump(std::ofstream& file);
    void PrintHelp(const CCommand& args);
};

Cvars::Cvars()
{
    this->hasLoaded = true;
}
int Cvars::Dump(std::ofstream& file)
{
    auto cmd = tier1->m_pConCommandList;
    auto count = 0;
    do {
        file << cmd->m_pszName;
        file << "[cvar_data]";

        auto IsCommand = reinterpret_cast<bool (*)(void*)>(Memory::VMT(cmd, Offsets::IsCommand));
        if (!IsCommand(cmd)) {
            auto cvar = reinterpret_cast<ConVar*>(cmd);
            file << cvar->m_pszDefaultValue;
        } else {
            file << "cmd";
        }
        file << "[cvar_data]";

        file << cmd->m_nFlags;
        file << "[cvar_data]";
        file << cmd->m_pszHelpString;
        file << "[end_of_cvar]";
        ++count;

    } while (cmd = cmd->m_pNext);

    return count;
}
void Cvars::ListAll()
{
    console->Msg("Commands:\n");
    for (auto& command : Command::list) {
        if (!!command && command->isRegistered) {
            auto ptr = command->ThisPtr();
            console->Print("\n%s\n", ptr->m_pszName);
            console->Msg("%s", ptr->m_pszHelpString);
        }
    }
    console->Msg("\nVariables:\n");
    for (auto& variable : Variable::list) {
        if (!variable) {
            continue;
        }

        auto ptr = variable->ThisPtr();
        if (variable->isRegistered) {
            console->Print("\n%s ", ptr->m_pszName);
            if (ptr->m_bHasMin) {
                console->Print("<number>\n");
            } else {
                console->Print("<string>\n");
            }
            console->Msg("%s", ptr->m_pszHelpString);
        } else if (variable->isReference) {
            console->Print("\n%s (unlocked)\n", ptr->m_pszName);
            if (std::strlen(ptr->m_pszHelpString) != 0) {
                console->Msg("%s\n", ptr->m_pszHelpString);
            }
        }
    }
}
void Cvars::PrintHelp(const CCommand& args)
{
    if (args.ArgC() != 2) {
        console->Print("Prints help string of cvar. Usage: help <cvar>\n");
        return;
    }

    auto cmd = reinterpret_cast<ConCommandBase*>(tier1->FindCommandBase(tier1->g_pCVar->ThisPtr(), args[1]));
    if (cmd) {
        auto IsCommand = reinterpret_cast<bool (*)(void*)>(Memory::VMT(cmd, Offsets::IsCommand));
        if (!IsCommand(cmd)) {
            auto cvar = reinterpret_cast<ConVar*>(cmd);
            console->Print("%s\n", cvar->m_pszName);
            console->Msg("Default: %s\n", cvar->m_pszDefaultValue);
            console->Msg("Flags: %i\n", cvar->m_nFlags);
            console->Msg("Description: %s\n", cvar->m_pszHelpString);
        } else {
            console->Print("%s\n", cmd->m_pszName);
            console->Msg("Flags: %i\n", cmd->m_nFlags);
            console->Msg("Description: %s\n", cmd->m_pszHelpString);
        }
    } else {
        console->Print("Unknown cvar name!\n");
    }
}

Cvars* cvars;
extern Cvars* cvars;
