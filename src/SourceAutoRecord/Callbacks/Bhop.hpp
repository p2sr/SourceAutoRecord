#pragma once
#include "Modules/Client.hpp"
#include "Modules/Tier1.hpp"

namespace Callbacks
{
    void IN_BhopDown(const void* ptr)
    {
        ConCommandArgs args(ptr);
        Client::KeyDown(Client::in_jump, (args.Count() > 1) ? args.At(1) : NULL);
    }
    void IN_BhopUp(const void* ptr)
    {
        ConCommandArgs args(ptr);
        Client::KeyUp(Client::in_jump, (args.Count() > 1) ? args.At(1) : NULL);
    }
}