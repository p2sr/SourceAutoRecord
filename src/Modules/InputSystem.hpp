#pragma once
#include "Command.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

#define BUTTON_CODE_INVALID -1
#define KEY_ESCAPE 70

namespace InputSystem {

Interface* g_InputSystem;

using _StringToButtonCode = int(__func*)(void* thisptr, const char* pString);
using _KeySetBinding = void(__cdecl*)(int keynum, const char* pBinding);

_StringToButtonCode StringToButtonCode;
_KeySetBinding KeySetBinding;

int GetButton(const char* pString)
{
    return StringToButtonCode(g_InputSystem->ThisPtr(), pString);
}

void Init()
{
    g_InputSystem = Interface::Create(MODULE("inputsystem"), "InputSystemVersion0");

    if (g_InputSystem) {
        StringToButtonCode = g_InputSystem->Original<_StringToButtonCode>(Offsets::StringToButtonCode);
    }

    auto unbind = Command("unbind");
    if (unbind.GetPtr()) {
        auto cc_unbind_callback = (uintptr_t)unbind.GetPtr()->m_pCommandCallback;
        KeySetBinding = Memory::Read<_KeySetBinding>(cc_unbind_callback + Offsets::Key_SetBinding);
    }
}
void Shutdown()
{
    Interface::Delete(g_InputSystem);
}
}
