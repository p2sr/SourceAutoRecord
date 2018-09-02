#include "InputSystem.hpp"

#include "Command.hpp"
#include "Interface.hpp"
#include "Module.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

int InputSystem::GetButton(const char* pString)
{
    return this->StringToButtonCode(this->g_InputSystem->ThisPtr(), pString);
}
bool InputSystem::Init()
{
    this->g_InputSystem = Interface::Create(this->Name(), "InputSystemVersion0");
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
