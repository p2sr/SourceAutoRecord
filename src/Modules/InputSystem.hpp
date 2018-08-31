#pragma once
#include "Module.hpp"

#include "Command.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

#define BUTTON_CODE_INVALID -1
#define KEY_ESCAPE 70

class InputSystem : public Module {
public:
    Interface* g_InputSystem;

    using _StringToButtonCode = int(__func*)(void* thisptr, const char* pString);
    using _KeySetBinding = void(__cdecl*)(int keynum, const char* pBinding);

    _StringToButtonCode StringToButtonCode;
    _KeySetBinding KeySetBinding;

public:
    int GetButton(const char* pString);

    bool Init() override;
    void Shutdown() override;
};

extern InputSystem* inputSystem;
