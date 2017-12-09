#pragma once
#include <fstream>
#include <cstdint>
#include <memory>
#include <Psapi.h>
#include <vector>
#include <Windows.h>

#define SAR_VERSION "1.1"
#define SAR_BUILD __TIME__ " " __DATE__

#define COL_GREEN Color(17, 224, 35)
#define COL_WHITE Color(255, 255, 255, 255)
#define COL_YELLOW Color(247, 235, 69)

#define INRANGE(x, a, b) (x >= a && x <= b)
#define getBits(x) (INRANGE((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xA): (INRANGE(x, '0', '9') ? x - '0': 0))
#define getByte(x) (getBits(x[0]) << 4 | getBits(x[1]))

struct Color {
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : Colors{ r, g, b, a } { }
	uint8_t Colors[4];
};

struct Signature {
	const char* Comment;
	const char* Bytes;
	const int Offset = 0;
};

struct ScanResult {
	uintptr_t Address;
	int Index = 0;
	bool Found = false;
};

struct Pattern {
	const char* Name;
	const char* Module;
	const std::vector<Signature> Signatures;
	ScanResult Result;

	const char* GetResult()
	{
		char result[256];
		if (Result.Found) {
			snprintf(result, sizeof(result), "Found %s at 0x%p in %s using %s!", Name, Result.Address, Module, Signatures[Result.Index].Comment);
		}
		else {
			snprintf(result, sizeof(result), "Failed to find %s!", Name);
		}
		return result;
	}
};

uintptr_t FindAddress(const uintptr_t& start_address, const uintptr_t& end_address, const char* target_pattern) {
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

	return NULL;
}

ScanResult Scan(Pattern& pattern) {
	MODULEINFO info = { 0 };

	if (GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(pattern.Module), &info, sizeof(MODULEINFO))) {
		const uintptr_t start = uintptr_t(info.lpBaseOfDll);
		const uintptr_t end = start + info.SizeOfImage;

		for (auto &signature : pattern.Signatures) {
			pattern.Result.Address = FindAddress(start, end, signature.Bytes);
			if (pattern.Result.Address == NULL) {
				pattern.Result.Index++;
				continue;
			}
			pattern.Result.Address += signature.Offset;
			pattern.Result.Found = true;
			break;
		}
	}
	return pattern.Result;
}

void* GetVirtualFunctionByIndex(void* ptr, int index)
{
	return (*((void***)ptr))[index];
}

int Error(std::string text, std::string title)
{
	MessageBoxA(0, text.c_str(), title.c_str(), MB_ICONERROR);
	return 1;
}