#pragma once
#include "Utils/Math.hpp"
#include "Utils/Memory.hpp"
#include "Utils/Platform.hpp"
#include "Utils/SDK.hpp"
#include <optional>

#ifndef _WIN32
#	include <unistd.h>
#endif

#include <string>

#define MODULE(name) name MODULE_EXTENSION

//CSV export
#define CSV_EXTENSION ".csv"
#define CSV_SEPARATOR ','
#define MICROSOFT_PLEASE_FIX_YOUR_SOFTWARE_SMHMYHEAD "sep=,"

namespace Utils {

	bool EndsWith(const std::string &str, const std::string &suffix);
	bool StartsWith(const char *str, const char *subStr);
	bool StartsWithInsens(const char *str, const char *subStr);
	bool ICompare(const std::string &a, const std::string &b);
	std::string ssprintf(const char *fmt, ...);
	uint8_t ConvertFromSrgb(uint8_t s);
	std::string GetSARPath();
	std::optional<Color> GetColor(const char *str, bool to_linear = false);
	Color HSVToRGB(float H, float S, float V);
	const char *ArgContinuation(const CCommand &args, int from);
}  // namespace Utils

#define REDECL(name) \
	decltype(name) name

#define SAFE_DELETE(ptr) \
	if (ptr) {              \
		delete ptr;            \
		ptr = nullptr;         \
	}

#if _WIN32
#	define GO_THE_FUCK_TO_SLEEP(ms) Sleep(ms)
#else
#	define GO_THE_FUCK_TO_SLEEP(ms) usleep((ms) * 1000)
#endif
