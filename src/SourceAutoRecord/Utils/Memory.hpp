#pragma once
#include <cstdint>
#include <cstring>
#include <dlfcn.h>
#include <fstream>
#include <link.h>
#include <memory>
#include <sys/uio.h>
#include <unistd.h>
#include <vector>

#define INRANGE(x, a, b) (x >= a && x <= b)
#define getBits(x) (INRANGE((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xA): (INRANGE(x, '0', '9') ? x - '0': 0))
#define getByte(x) (getBits(x[0]) << 4 | getBits(x[1]))

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

			module.lpBaseOfDll = info->dlpi_addr + info->dlpi_phdr[0].p_paddr;
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
				snprintf
				(
					result.Message,
					sizeof(result.Message),
					"Found %s at %p in %s!",
					pattern->Description,
					(void*)result.Address,
					pattern->Module
				);
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