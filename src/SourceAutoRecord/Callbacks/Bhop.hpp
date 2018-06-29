#pragma once
#include "Modules/Client.hpp"
#include "Modules/Tier1.hpp"

namespace Callbacks {

void IN_BhopDown(const CCommand& args)
{
    Client::KeyDown(Client::in_jump, (args.ArgC() > 1) ? args[1] : NULL);
}
void IN_BhopUp(const CCommand& args)
{
    Client::KeyUp(Client::in_jump, (args.ArgC() > 1) ? args[1] : NULL);
}
}