#pragma once
#include "Command.hpp"
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

#define BUTTON_CODE_INVALID -1
#define KEY_ESCAPE 70

namespace InputSystem {

VMT g_InputSystem;

using _StringToButtonCode = int(__func*)(void* thisptr, const char* pString);
using _KeySetBinding = void(__cdecl*)(int keynum, const char* pBinding);

_StringToButtonCode StringToButtonCode;
_KeySetBinding KeySetBinding;

int GetButton(const char* pString)
{
    return StringToButtonCode(g_InputSystem->GetThisPtr(), pString);
}

void Hook()
{
    CREATE_VMT(Interfaces::IInputSystem, g_InputSystem)
    {
        StringToButtonCode = g_InputSystem->GetOriginal<_StringToButtonCode>(Offsets::StringToButtonCode);
    }

    auto unbind = Command("unbind");
    if (unbind.GetPtr()) {
        auto cc_unbind_callback = (uintptr_t)unbind.GetPtr()->m_pCommandCallback;
        KeySetBinding = Memory::Read<_KeySetBinding>(cc_unbind_callback + Offsets::Key_SetBinding);
    }
}
void Unhook()
{
    DELETE_VMT(g_InputSystem);
}
}
