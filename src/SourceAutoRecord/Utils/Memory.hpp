#pragma once
#include <cstring>
#include <fstream>
#include <memory>
#include <vector>

#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
// Last
#include <psapi.h>
#else
#include <cstdint>
#include <dlfcn.h>
#include <link.h>
#include <sys/uio.h>
#include <unistd.h>
#define MAX_PATH 4096
#endif

#define INRANGE(x, a, b) (x >= a && x <= b)
#define getBits(x) (INRANGE((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xA) : (INRANGE(x, '0', '9') ? x - '0' : 0))
#define getByte(x) (getBits(x[0]) << 4 | getBits(x[1]))

namespace Memory {

struct Pattern {
    const char* Module;
    const char* Name;

    const char* Version;
    const char* Description;
    const char* Bytes;
    int Offset;

    bool IsSet;

    void SetSignature(const char* version, const char* description)
    {
        Version = version;
        Description = description;
    }
    void SetSignature(const char* version, const char* description, const char* bytes, int offset = 0)
    {
        Version = version;
        Description = description;
        Bytes = bytes;
        Offset = offset;
        IsSet = true;
    }
};

struct ScanResult {
    uintptr_t Address;
    int Index = 0;
    bool Found = false;
    char Message[256];
};

static uintptr_t FindAddress(const uintptr_t& start_address, const uintptr_t& end_address, const char* target_pattern)
{
    const char* pattern = target_pattern;
    uintptr_t first_match = 0;

    for (uintptr_t position = start_address; position < end_address; position++) {
        if (!*pattern)
            return first_match;

        const uint8_t pattern_current = *reinterpret_cast<const uint8_t*>(pattern);
        const uint8_t memory_current = *reinterpret_cast<const uint8_t*>(position);

        if (pattern_current == '\?' || memory_current == getByte(pattern)) {
            if (!first_match)
                first_match = position;

            if (!pattern[2])
                return first_match;

            pattern += (pattern_current != '\?') ? 3 : 2;
        } else {
            pattern = target_pattern;
            first_match = 0;
        }
    }
    return 0;
}

struct ModuleInfo {
    char name[MAX_PATH];
    uintptr_t base;
    uintptr_t size;
    char path[MAX_PATH];
};

#ifdef _WIN32
static bool TryGetModule(const char* moduleName, ModuleInfo* info)
{
    auto mHandle = GetModuleHandleA(moduleName);

    char buffer[MAX_PATH];
    if (!GetModuleFileName(mHandle, buffer, sizeof(buffer)))
        return false;

    auto temp = MODULEINFO();
    auto pHandle = GetCurrentProcess();
    if (!GetModuleInformation(pHandle, mHandle, &temp, sizeof(temp)))
        return false;

    auto name = std::string(buffer);
    auto index = name.find_last_of("\\/");
    name = name.substr(index + 1, name.length() - index);

    snprintf(info->name, sizeof(info->name), "%s", name.c_str());
    info->base = (uintptr_t)temp.lpBaseOfDll;
    info->size = (uintptr_t)temp.SizeOfImage;
    snprintf(info->path, sizeof(info->path), "%s", buffer);

    return true;
}
#else
namespace Cache {
    static std::vector<ModuleInfo> Modules;
}

static bool TryGetModule(const char* moduleName, ModuleInfo* info)
{
    if (Cache::Modules.size() == 0) {
        dl_iterate_phdr([](struct dl_phdr_info* info, size_t, void*) {
            auto module = ModuleInfo();

            std::string temp = std::string(info->dlpi_name);
            int index = temp.find_last_of("\\/");
            temp = temp.substr(index + 1, temp.length() - index);
            snprintf(module.name, sizeof(module.name), "%s", temp.c_str());

            module.base = info->dlpi_addr + info->dlpi_phdr[0].p_paddr;
            module.size = info->dlpi_phdr[0].p_memsz;
            strcpy(module.path, info->dlpi_name);

            Cache::Modules.push_back(module);
            return 0;
        },
            nullptr);
    }

    for (ModuleInfo& item : Cache::Modules) {
        if (!std::strcmp(item.name, moduleName)) {
            *info = item;
            return true;
        }
    }

    return false;
}
#endif

static ScanResult Scan(const char* moduleName, const char* pattern, int offset = 0)
{
    auto result = ScanResult();
    auto info = ModuleInfo();

    if (TryGetModule(moduleName, &info)) {
        const uintptr_t start = uintptr_t(info.base);
        const uintptr_t end = start + info.size;
        result.Address = FindAddress(start, end, pattern);
        if (result.Address) {
            result.Found = true;
            result.Address += offset;
        }
    }

    return result;
}

static ScanResult Scan(Pattern* pattern)
{
    auto result = ScanResult();
    auto info = ModuleInfo();

    if (TryGetModule(pattern->Module, &info)) {
        const uintptr_t start = uintptr_t(info.base);
        const uintptr_t end = start + info.size;

        if (pattern->IsSet) {
            result.Address = FindAddress(start, end, pattern->Bytes);
            if (result.Address) {
                result.Address += pattern->Offset;
                result.Found = true;
                snprintf(result.Message,
                    sizeof(result.Message),
                    "Found %s at %p in %s!",
                    pattern->Description,
                    (void*)result.Address,
                    pattern->Module);
            } else {
                snprintf(result.Message, sizeof(result.Message), "Failed to find %s!", pattern->Description);
            }
        } else {
            snprintf(result.Message, sizeof(result.Message), "Ignored %s!", pattern->Name);
        }
    }

    return result;
}

static void* GetModuleHandleByName(const char* moduleName)
{
    auto info = ModuleInfo();
#ifdef _WIN32
    return (TryGetModule(moduleName, &info)) ? GetModuleHandleA(info.path) : nullptr;
#else
    return (TryGetModule(moduleName, &info)) ? dlopen(info.path, RTLD_NOLOAD | RTLD_NOW) : nullptr;
#endif
}

static void CloseModuleHandle(void* moduleHandle)
{
#ifndef _WIN32
    dlclose(moduleHandle);
#endif
}

static void* GetSymbolAddress(void* moduleHandle, const char* symbolName)
{
#ifdef _WIN32
    return (void*)GetProcAddress((HMODULE)moduleHandle, symbolName);
#else
    return dlsym(moduleHandle, symbolName);
#endif
}

static const char* GetModulePath(const char* moduleName)
{
    auto info = ModuleInfo();
    return (TryGetModule(moduleName, &info)) ? std::string(info.path).c_str() : nullptr;
}

static std::string GetProcessName()
{
#ifdef _WIN32
    char temp[MAX_PATH];
    GetModuleFileName(NULL, temp, sizeof(temp));
#else
    char link[32];
    char temp[MAX_PATH] = { 0 };
    sprintf(link, "/proc/%d/exe", getpid());
    readlink(link, temp, sizeof(temp));
#endif

    auto proc = std::string(temp);
    auto index = proc.find_last_of("\\/");
    proc = proc.substr(index + 1, proc.length() - index);

    return proc;
}

template <typename T = void*>
inline T VMT(void* ptr, int index)
{
    return reinterpret_cast<T>((*((void***)ptr))[index]);
}

template <typename T = uintptr_t>
static T ReadAbsoluteAddress(uintptr_t source)
{
    auto rel = *reinterpret_cast<int*>(source);
    return (T)(source + rel + sizeof(rel));
}

template <typename T = void*>
inline T Deref(uintptr_t address)
{
    return *reinterpret_cast<T*>(address);
}

template <typename T = void*>
inline T DerefDeref(uintptr_t address)
{
    return **reinterpret_cast<T**>(address);
}

static uintptr_t ToAbsoluteAddress(const char* moduleName, int relative)
{
    auto info = ModuleInfo();
    return (TryGetModule(moduleName, &info)) ? info.base + relative : 0;
}
}
