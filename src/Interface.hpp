#pragma once
#include "Utils/Memory.hpp"

class Interface {
public:
    uintptr_t** baseclass;
    uintptr_t* vtable;
    int vtableSize;

private:
    bool isHooked;
    std::unique_ptr<uintptr_t[]> copy;

public:
    Interface();
    Interface(void* baseclass, bool copyVtable = true, bool autoHook = true);
    ~Interface();
    void CopyVtable();
    void EnableHooks();
    void DisableHooks();
    template <typename T = uintptr_t>
    T Original(int index, bool readJmp = false);
    template <typename T = uintptr_t>
    T Hooked(int index);
    template <typename T = uintptr_t>
    T Current(int index);
    template <typename T = uintptr_t, typename U = void*>
    bool Hook(T detour, U& original, int index);
    bool Unhook(int index);
    inline void* ThisPtr() { return reinterpret_cast<void*>(this->baseclass); }
    static Interface* Create(void* ptr, bool copyVtable = true, bool autoHook = true);
    static Interface* Create(const char* filename, const char* interfaceSymbol, bool copyVtable = true, bool autoHook = true);
    static void Delete(Interface* ptr);
};

template <typename T>
T Interface::Original(int index, bool readJmp)
{
    if (readJmp) {
        auto source = this->vtable[index] + 1;
        auto rel = *reinterpret_cast<uintptr_t*>(source);
        return (T)(source + rel + sizeof(rel));
    }
    return (T)this->vtable[index];
}
template <typename T>
T Interface::Hooked(int index)
{
    return (T)this->copy[index];
}
template <typename T>
T Interface::Current(int index)
{
    return (T)(*this->baseclass)[index];
}
template <typename T, typename U>
bool Interface::Hook(T detour, U& original, int index)
{
    if (index >= 0 && index < this->vtableSize) {
        this->copy[index] = reinterpret_cast<uintptr_t>(detour);
        original = this->Original<U>(index);
        return true;
    }
    return false;
}
