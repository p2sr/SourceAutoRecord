#pragma once
#include "Command.hpp"
#include "Interface.hpp"
#include "Module.hpp"
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

int InputSystem::GetButton(const char* pString)
{
    return this->StringToButtonCode(this->g_InputSystem->ThisPtr(), pString);
}
bool InputSystem::Init()
{
    this->g_InputSystem = Interface::Create(MODULE("inputsystem"), "InputSystemVersion0");
    if (this->g_InputSystem) {
        this->StringToButtonCode = this->g_InputSystem->Original<_StringToButtonCode>(Offsets::StringToButtonCode);
    }

    auto unbind = Command("unbind");
    if (!!unbind) {
        auto cc_unbind_callback = (uintptr_t)unbind.ThisPtr()->m_pCommandCallback;
        this->KeySetBinding = Memory::Read<_KeySetBinding>(cc_unbind_callback + Offsets::Key_SetBinding);
    }

    return this->hasLoaded = this->g_InputSystem && !!unbind;
}
void InputSystem::Shutdown()
{
    Interface::Delete(this->g_InputSystem);
}

InputSystem* inputSystem;
extern InputSystem* inputSystem;
