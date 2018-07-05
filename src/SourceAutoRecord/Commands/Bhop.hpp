#pragma once
#include "Modules/Client.hpp"

#include "Command.hpp"

namespace Commands {

void IN_BhopDown(const CCommand& args) { Client::KeyDown(Client::in_jump, (args.ArgC() > 1) ? args[1] : NULL); }
void IN_BhopUp(const CCommand& args) { Client::KeyUp(Client::in_jump, (args.ArgC() > 1) ? args[1] : NULL); }

Command startbhop("+bhop", IN_BhopDown, "Sends client a keydown event for the in_jump state.");
Command endbhop("-bhop", IN_BhopUp, "Sends client a keyup event for the in_jump state.");
}