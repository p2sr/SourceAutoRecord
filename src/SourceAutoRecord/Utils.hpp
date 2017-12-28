#pragma once
#include <cstdint>
#include <fstream>
#include <memory>
#include <Psapi.h>
#include <vector>
#include <Windows.h>

#include "Game.hpp"

#define SAR_VERSION "1.5"
#define SAR_BUILD __TIME__ " " __DATE__
#define SAR_COLOR Color(247, 235, 69)

#define COL_ACTIVE Color(110, 247, 76)
#define COL_DEFAULT Color(255, 255, 255, 255)

#define INRANGE(x, a, b) (x >= a && x <= b)
#define getBits(x) (INRANGE((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xA): (INRANGE(x, '0', '9') ? x - '0': 0))
#define getByte(x) (getBits(x[0]) << 4 | getBits(x[1]))

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

struct Signature {
	const char* Version;
	const char* Name;
	const char* Bytes;
	const int Offset = 0;
};

struct Pattern {
	const char* Module;
	const char* Name;
	std::vector<Signature> Signatures;
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
	return NULL;
}

ScanResult Scan(const char* moduleName, const char* pattern, int offset = 0)
{
	auto info = MODULEINFO();
	auto result = ScanResult();

	if (GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(moduleName), &info, sizeof(MODULEINFO))) {
		const uintptr_t start = uintptr_t(info.lpBaseOfDll);
		const uintptr_t end = start + info.SizeOfImage;
		result.Address = FindAddress(start, end, pattern);
		if (result.Address != NULL) {
			result.Found = true;
			result.Address += offset;
		}
	}
	return result;
}

ScanResult Scan(Pattern* pattern)
{
	auto info = MODULEINFO();
	auto result = ScanResult();

	if (GetModuleInformation(GetCurrentProcess(), GetModuleHandleA((*pattern).Module), &info, sizeof(MODULEINFO))) {
		const uintptr_t start = uintptr_t(info.lpBaseOfDll);
		const uintptr_t end = start + info.SizeOfImage;

		if ((int)pattern->Signatures.size() >= Game::Version + 1) {
			auto signature = pattern->Signatures[Game::Version];
			result.Address = FindAddress(start, end, signature.Bytes);
			if (result.Address != NULL) {
				result.Address += signature.Offset;
				result.Found = true;
				snprintf(result.Message, sizeof(result.Message), "Found %s at 0x%p in %s!", signature.Name, (void*)result.Address, pattern->Module);
			}
			else {
				snprintf(result.Message, sizeof(result.Message), "Failed to find %s!", signature.Name);
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

int Error(std::string text, std::string title)
{
	MessageBoxA(0, text.c_str(), title.c_str(), MB_ICONERROR);
	return 1;
}

bool DoNothingAt(uintptr_t address, int count)
{
	BYTE nop[1] = { 0x90 };

	for (int i = 0; i < count; i++) {
		if (!WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(address + i), nop, 1, 0)) {
			return false;
		}
	}
	return true;
}