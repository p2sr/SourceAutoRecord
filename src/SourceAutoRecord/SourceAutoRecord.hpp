#pragma once
#include "vmthook/vmthook.h"

#include "Modules/Console.hpp"

#include "Patterns.hpp"
#include "Utils.hpp"

#define SAR_VERSION "1.7"
#define SAR_BUILD __TIME__ " " __DATE__
#define SAR_COLOR Color(247, 235, 69)
#define COL_ACTIVE Color(110, 247, 76)
#define COL_DEFAULT Color(255, 255, 255, 255)

namespace SAR {

Memory::ScanResult Find(const char* pattern)
{
    auto result = Memory::Scan(Patterns::Get(pattern));
    if (result.Found) {
        Console::DevMsg("SAR: %s\n", result.Message);
    } else {
        Console::DevWarning("SAR: %s\n", result.Message);
    }
    return result;
}
bool NewVMT(void* ptr, std::unique_ptr<VMTHook>& hook)
{
    if (ptr) {
        hook = std::make_unique<VMTHook>(ptr);
        Console::DevMsg("SAR: Created new VMT for %p at %p with %i functions.\n", ptr, hook->GetThisPtr(), hook->GetTotalFunctions());
        return true;
    }

    Console::DevWarning("SAR: Skipped creating new VMT for %p.\n", ptr);
    return false;
}
void DeleteVMT(std::unique_ptr<VMTHook>& hook)
{
    hook.release();
}
}