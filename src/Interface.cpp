#include "Interface.hpp"

#include <cstring>

#include "Modules/Console.hpp"

#include "Utils/Memory.hpp"
#include "Utils/SDK.hpp"

#define CreateInterfaceInternal_Offset 5
#ifdef _WIN32
#define s_pInterfaceRegs_Offset 6
#else
#define s_pInterfaceRegs_Offset 11
#endif

Interface::Interface()
    : baseclass(nullptr)
    , vtable(nullptr)
    , vtableSize(0)
    , isHooked(false)
    , copy(nullptr)
{
}
Interface::Interface(void* baseclass, bool copyVtable, bool autoHook)
    : Interface()
{
    this->baseclass = reinterpret_cast<uintptr_t**>(baseclass);
    this->vtable = *this->baseclass;

    while (this->vtable[this->vtableSize]) {
        ++this->vtableSize;
    }

    if (copyVtable) {
        this->CopyVtable();
        if (autoHook) {
            this->EnableHooks();
        }
    }
}
Interface::~Interface()
{
    this->DisableHooks();
    if (this->copy) {
        this->copy.reset();
    }
}
void Interface::CopyVtable()
{
    if (!this->copy) {
        this->copy = std::make_unique<uintptr_t[]>(this->vtableSize + 1);
        std::memcpy(this->copy.get(), this->vtable - 1, sizeof(uintptr_t) + this->vtableSize * sizeof(uintptr_t));
    }
}
void Interface::EnableHooks()
{
    if (!this->isHooked) {
        *this->baseclass = this->copy.get() + 1;
        this->isHooked = true;
    }
}
void Interface::DisableHooks()
{
    if (this->isHooked) {
        *this->baseclass = this->vtable;
        this->isHooked = false;
    }
}
bool Interface::Unhook(int index)
{
    if (index >= 0 && index < this->vtableSize) {
        this->copy[index + 1] = this->vtable[index];
        return true;
    }
    return false;
}
Interface* Interface::Create(void* ptr, bool copyVtable, bool autoHook)
{
    return (ptr) ? new Interface(ptr, copyVtable, autoHook) : nullptr;
}
Interface* Interface::Create(const char* filename, const char* interfaceSymbol, bool copyVtable, bool autoHook)
{
    auto ptr = Interface::GetPtr(filename, interfaceSymbol);
    return (ptr) ? new Interface(ptr, copyVtable, autoHook) : nullptr;
}
void Interface::Delete(Interface* ptr)
{
    if (ptr) {
        delete ptr;
        ptr = nullptr;
    }
}
void* Interface::GetPtr(const char* filename, const char* interfaceSymbol)
{
    auto handle = Memory::GetModuleHandleByName(filename);
    if (!handle) {
        console->DevWarning("SAR: Failed to open module %s!\n", filename);
        return nullptr;
    }

    auto CreateInterface = Memory::GetSymbolAddress<uintptr_t>(handle, "CreateInterface");
    Memory::CloseModuleHandle(handle);

    if (!CreateInterface) {
        console->DevWarning("SAR: Failed to find symbol CreateInterface for %s!\n", filename);
        return nullptr;
    }

#ifdef _WIN32
    auto obe = Memory::Deref<uint8_t>(CreateInterface) == 0xE9; // jmp
#else
    auto obe = false;
#endif

    auto CreateInterfaceInternal = Memory::Read(CreateInterface + (obe ? 1 : CreateInterfaceInternal_Offset));
    auto s_pInterfaceRegs = Memory::DerefDeref<InterfaceReg*>(CreateInterfaceInternal + (obe ? 3 : s_pInterfaceRegs_Offset));

    void* result = nullptr;
    for (auto& current = s_pInterfaceRegs; current; current = current->m_pNext) {
        if (std::strncmp(current->m_pName, interfaceSymbol, std::strlen(interfaceSymbol)) == 0) {
            result = current->m_CreateFn();
            //console->DevMsg("SAR: Found interface %s at %p in %s!\n", current->m_pName, result, filename);
            break;
        }
    }

    if (!result) {
        console->DevWarning("SAR: Failed to find interface with symbol %s in %s!\n", interfaceSymbol, filename);
        return nullptr;
    }
    return result;
}
