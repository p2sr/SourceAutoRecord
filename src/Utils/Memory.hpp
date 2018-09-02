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

struct ModuleInfo {
    char name[MAX_PATH];
    uintptr_t base;
    uintptr_t size;
    char path[MAX_PATH];
};

static std::vector<ModuleInfo> moduleList;

static bool TryGetModule(const char* moduleName, ModuleInfo* info)
{
    if (moduleList.size() == 0) {
#ifdef _WIN32
        HMODULE hMods[1024];
        HANDLE pHandle = GetCurrentProcess();
        DWORD cbNeeded;
        if (EnumProcessModules(pHandle, hMods, sizeof(hMods), &cbNeeded)) {
            for (unsigned i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
                char buffer[MAX_PATH];
                if (!GetModuleFileName(hMods[i], buffer, sizeof(buffer)))
                    continue;

                auto modinfo = MODULEINFO();
                if (!GetModuleInformation(pHandle, hMods[i], &modinfo, sizeof(modinfo)))
                    continue;

                auto module = ModuleInfo();

                auto temp = std::string(buffer);
                auto index = temp.find_last_of("\\/");
                temp = temp.substr(index + 1, temp.length() - index);

                snprintf(module.name, sizeof(module.name), "%s", temp.c_str());
                module.base = (uintptr_t)modinfo.lpBaseOfDll;
                module.size = (uintptr_t)modinfo.SizeOfImage;
                snprintf(module.path, sizeof(module.path), "%s", buffer);

                moduleList.push_back(module);
            }
        }

#else
        dl_iterate_phdr([](struct dl_phdr_info* info, size_t, void*) {
            auto module = ModuleInfo();

            std::string temp = std::string(info->dlpi_name);
            int index = temp.find_last_of("\\/");
            temp = temp.substr(index + 1, temp.length() - index);
            snprintf(module.name, sizeof(module.name), "%s", temp.c_str());

            module.base = info->dlpi_addr + info->dlpi_phdr[0].p_paddr;
            module.size = info->dlpi_phdr[0].p_memsz;
            strcpy(module.path, info->dlpi_name);

            moduleList.push_back(module);
            return 0;
        },
            nullptr);
#endif
    }

    for (ModuleInfo& item : moduleList) {
        if (!std::strcmp(item.name, moduleName)) {
            if (info) {
                *info = item;
            }
            return true;
        }
    }

    return false;
}

static const char* GetModulePath(const char* moduleName)
{
    auto info = ModuleInfo();
    return (TryGetModule(moduleName, &info)) ? std::string(info.path).c_str() : nullptr;
}

template <typename T = uintptr_t>
static T Absolute(const char* moduleName, int relative)
{
    auto info = ModuleInfo();
    return (TryGetModule(moduleName, &info)) ? (T)(info.base + relative) : (T)0;
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

template <typename T = void*>
static T GetSymbolAddress(void* moduleHandle, const char* symbolName)
{
#ifdef _WIN32
    return (T)GetProcAddress((HMODULE)moduleHandle, symbolName);
#else
    return (T)dlsym(moduleHandle, symbolName);
#endif
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
inline T Read(uintptr_t source)
{
    auto rel = *reinterpret_cast<int*>(source);
    return (T)(source + rel + sizeof(rel));
}

template <typename T = uintptr_t>
static void Read(uintptr_t source, T* destination)
{
    auto rel = *reinterpret_cast<int*>(source);
    *destination = (T)(source + rel + sizeof(rel));
}

template <typename T = void*>
inline T Deref(uintptr_t source)
{
    return *reinterpret_cast<T*>(source);
}

template <typename T = void*>
static void Deref(uintptr_t source, T* destination)
{
    *destination = *reinterpret_cast<T*>(source);
}

template <typename T = void*>
inline T DerefDeref(uintptr_t source)
{
    return **reinterpret_cast<T**>(source);
}

template <typename T = void*>
static void DerefDeref(uintptr_t source, T* destination)
{
    *destination = **reinterpret_cast<T**>(source);
}

static uintptr_t FindAddress(const uintptr_t start, const uintptr_t end, const char* target)
{
    const char* pattern = target;
    uintptr_t result = 0;

    for (auto position = start; position < end; ++position) {
        if (!*pattern)
            return result;

        auto match = *reinterpret_cast<const uint8_t*>(pattern);
        auto byte = *reinterpret_cast<const uint8_t*>(position);

        if (match == '\?' || byte == getByte(pattern)) {
            if (!result)
                result = position;

            if (!pattern[2])
                return result;

            pattern += (match != '\?') ? 3 : 2;
        } else {
            pattern = target;
            result = 0;
        }
    }
    return 0;
}

static uintptr_t Scan(const char* moduleName, const char* pattern, int offset = 0)
{
    uintptr_t result = 0;

    auto info = ModuleInfo();
    if (TryGetModule(moduleName, &info)) {
        auto start = uintptr_t(info.base);
        auto end = start + info.size;
        result = FindAddress(start, end, pattern);
        if (result) {
            result += offset;
        }
    }
    return result;
}
}
