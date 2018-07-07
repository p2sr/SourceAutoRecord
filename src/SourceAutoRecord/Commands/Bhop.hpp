#pragma once
#include "Modules/Client.hpp"

#include "Command.hpp"

// The Stanley Parable only
namespace Commands {

void IN_BhopDown(const CCommand& args) { Client::KeyDown(Client::in_jump, (args.ArgC() > 1) ? args[1] : NULL); }
void IN_BhopUp(const CCommand& args) { Client::KeyUp(Client::in_jump, (args.ArgC() > 1) ? args[1] : NULL); }

Command startbhop("+bhop", IN_BhopDown, "Client sends a keydown event for the in_jump state.");
Command endbhop("-bhop", IN_BhopUp, "Client sends a keyup event for the in_jump state.");
}

CON_COMMAND(sar_anti_anti_cheat, "Sets sv_cheats to 1.\n")
{
    Cheats::sv_cheats.ptr->m_nValue = 1;
}