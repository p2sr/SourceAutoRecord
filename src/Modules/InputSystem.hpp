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
    Interface* g_InputSystem = nullptr;

    using _StringToButtonCode = int(__func*)(void* thisptr, const char* pString);
    using _KeySetBinding = void(__cdecl*)(int keynum, const char* pBinding);

    _StringToButtonCode StringToButtonCode = nullptr;
    _KeySetBinding KeySetBinding = nullptr;

public:
    int GetButton(const char* pString);

    // CInputSystem::SleepUntilInput
    DECL_DETOUR(SleepUntilInput, int nMaxSleepTimeMS);

    bool Init() override;
    void Shutdown() override;
    const char* Name() override { return MODULE("inputsystem"); }
};

extern InputSystem* inputSystem;
