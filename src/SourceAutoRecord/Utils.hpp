#pragma once
#include <cmath>
#include <cstdint>
#include <cstring>
#include <dlfcn.h>
#include <fstream>
#include <link.h>
#include <memory>
#include <sys/uio.h>
#include <unistd.h>
#include <vector>

#include "vmthook/vmthook.h"

#define SAR_VERSION "1.5"
#define SAR_BUILD __TIME__ " " __DATE__
#define SAR_COLOR Color(247, 235, 69)

#define COL_ACTIVE Color(110, 247, 76)
#define COL_DEFAULT Color(255, 255, 255, 255)

#define INRANGE(x, a, b) (x >= a && x <= b)
#define getBits(x) (INRANGE((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xA): (INRANGE(x, '0', '9') ? x - '0': 0))
#define getByte(x) (getBits(x[0]) << 4 | getBits(x[1]))

#define __cdecl __attribute__((__cdecl__))
#define __stdcall __attribute__((__stdcall__))
#define __fastcall __attribute__((__fastcall__))
#define __thiscall __attribute__((__thiscall__))

struct Vector {
	float x, y, z;
	float Length() {
		return sqrt(x * x + y * y + z * z);
	}
	float Length2D() {
		return sqrt(x * x + y * y);
	}
};

struct QAngle {
	float x, y, z;
};

struct Color {
	Color() {
		*((int *)this) = 255;
	}
	Color(int _r, int _g, int _b) {
		SetColor(_r, _g, _b, 255);
	}
	Color(int _r, int _g, int _b, int _a) {
		SetColor(_r, _g, _b, _a);
	}
	void SetColor(int _r, int _g, int _b, int _a = 255) {
		_color[0] = (unsigned char)_r;
		_color[1] = (unsigned char)_g;
		_color[2] = (unsigned char)_b;
		_color[3] = (unsigned char)_a;
	}
	inline int r() const { return _color[0]; }
	inline int g() const { return _color[1]; }
	inline int b() const { return _color[2]; }
	inline int a() const { return _color[3]; }
	unsigned char _color[4];
};

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

uintptr_t FindAddress(const uintptr_t& start_address, const uintptr_t& end_address, const char* target_pattern)
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
		}
		else {
			pattern = target_pattern;
			first_match = 0;
		}
	}
	return 0;
}

struct MODULEINFO {
	char moduleName[64];
	uintptr_t lpBaseOfDll;
	uintptr_t SizeOfImage;
};

namespace Cache
{
	std::vector<MODULEINFO> Modules;
}

bool GetModuleInformation(const char* moduleName, MODULEINFO* moduleInfo) {
	if (Cache::Modules.size() == 0) {
		dl_iterate_phdr([] (struct dl_phdr_info* info, size_t, void*) {
			auto module = MODULEINFO();

			std::string temp = std::string(info->dlpi_name);
			int index = temp.find_last_of("\\/");
			temp = temp.substr(index + 1, temp.length() - index);
			snprintf(module.moduleName, sizeof(module.moduleName), "%s", temp.c_str());

			module.lpBaseOfDll = info->dlpi_addr;
			module.SizeOfImage = info->dlpi_phdr[0].p_memsz;
			
			Cache::Modules.push_back(module);
			return 0;
		}, nullptr);
	}

	for (MODULEINFO& item: Cache::Modules) {
		if (!strcasestr(item.moduleName, moduleName))
			continue;
		*moduleInfo = item;
		return true;
	}

	return false;
}

ScanResult Scan(const char* moduleName, const char* pattern, int offset = 0)
{
	auto result = ScanResult();
	auto info = MODULEINFO();

	if (GetModuleInformation(moduleName, &info)) {
		const uintptr_t start = uintptr_t(info.lpBaseOfDll);
		const uintptr_t end = start + info.SizeOfImage;
		result.Address = FindAddress(start, end, pattern);
		if (result.Address) {
			result.Found = true;
			result.Address += offset;
		}
	}

	return result;
}

ScanResult Scan(Pattern* pattern)
{
	auto result = ScanResult();
	auto info = MODULEINFO();

	if (GetModuleInformation(pattern->Module, &info)) {
		const uintptr_t start = uintptr_t(info.lpBaseOfDll);
		const uintptr_t end = start + info.SizeOfImage;

		if (pattern->IsSet) {
			result.Address = FindAddress(start, end, pattern->Bytes);
			if (result.Address) {
				result.Address += pattern->Offset;
				result.Found = true;
				snprintf(result.Message, sizeof(result.Message), "Found %s at 0x%p in %s!", pattern->Description, (void*)result.Address, pattern->Module);
			}
			else {
				snprintf(result.Message, sizeof(result.Message), "Failed to find %s!", pattern->Description);
			}
		}
		else {
			snprintf(result.Message, sizeof(result.Message), "Ignored %s!", pattern->Name);
		}
	}

	return result;
}

void* GetVirtualFunctionByIndex(void* ptr, int index)
{
	return (*((void***)ptr))[index];
}

int NewReferenceAt(void* source, void* destination)
{
	char buffer[4];
	memcpy(buffer, &destination, 4);

	struct iovec local[1];
	struct iovec remote[1];

	local[0].iov_base = buffer;
	local[0].iov_len = 4;
	remote[0].iov_base = source;
	remote[0].iov_len = 4;

	return process_vm_writev(getpid(), local, 1, remote, 1, 0);
}

typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);

void* GetInterface(const char* filename, const char* version) {
	auto handle = dlopen(filename, RTLD_NOLOAD | RTLD_NOW);
	if (!handle) return nullptr;

	auto factory = dlsym(handle, "CreateInterface");
	if (!factory) return nullptr;

	return ((CreateInterfaceFn)(factory))(version, nullptr);
}