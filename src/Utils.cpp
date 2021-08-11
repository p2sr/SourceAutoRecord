#include "Utils.hpp"

#include <cctype>
#include <cstdarg>
#include <cstring>
#include <string>

#ifdef _WIN32
// clang-format off
#	include <Windows.h>
#	include <ImageHlp.h>
// clang-format on
#else
#	include <dlfcn.h>
#endif

bool Utils::EndsWith(const std::string &str, const std::string &suffix) {
	return str.size() >= suffix.size() && !str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}
bool Utils::StartsWith(const char *str, const char *subStr) {
	return std::strlen(str) >= std::strlen(subStr) && std::strstr(str, subStr) == str;
}
bool Utils::ICompare(const std::string &a, const std::string &b) {
	return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) {
		return std::tolower(a) == std::tolower(b);
	});
};
std::string Utils::ssprintf(const char *fmt, ...) {
	va_list ap1, ap2;
	va_start(ap1, fmt);
	va_copy(ap2, ap1);
	size_t sz = vsnprintf(NULL, 0, fmt, ap1) + 1;
	va_end(ap1);
	char *buf = (char *)malloc(sz);
	vsnprintf(buf, sz, fmt, ap2);
	va_end(ap2);
	std::string str(buf);
	free(buf);
	return str;
}
int Utils::ConvertFromSrgb(int s) {
	double s_ = (double)s / 255;
	double l = s <= 0.04045 ? s_ / 12.92 : pow((s_ + 0.055) / 1.055, 2.4);
	return (int)(l * 255);
}
std::string Utils::GetSARPath() {
#ifdef _WIN32
	SymInitialize(GetCurrentProcess(), 0, true);
	DWORD module = SymGetModuleBase(GetCurrentProcess(), (DWORD)&Utils::GetSARPath);
	char filename[MAX_PATH + 1];
	GetModuleFileNameA((HMODULE)module, filename, MAX_PATH);
	SymCleanup(GetCurrentProcess());
	return std::string(filename);
#else
	Dl_info info;
	dladdr((void *)&Utils::GetSARPath, &info);
	return std::string(info.dli_fname);
#endif
}
std::optional<Color> Utils::GetColor(const char *str, bool to_linear) {
#define RET(r, g, b, a) \
	return to_linear ? Color{ConvertFromSrgb(r), ConvertFromSrgb(g), ConvertFromSrgb(b), a} : Color { r, g, b, a }

	while (isspace(*str)) ++str;
	size_t len = strlen(str);
	for (size_t i = len - 1; i; --i) {
		if (!isspace(str[i])) break;
		--len;
	}

	bool had_hash = str[0] == '#';
	if (had_hash) ++str, --len;

	int r, g, b, a;
	int end;

	if (len == 8 && sscanf(str, "%2x%2x%2x%2x%n", &r, &g, &b, &a, &end) == 4 && end >= 8) {
		RET(r, g, b, a);
	}

	if (len == 6 && sscanf(str, "%2x%2x%2x%n", &r, &g, &b, &end) == 3 && end >= 6) {
		RET(r, g, b, 255);
	}

	if (had_hash) return {};

	if (sscanf(str, "%u %u %u %u%n", &r, &g, &b, &a, &end) == 4 && end >= len) {
		RET(r, g, b, a);
	}

	if (sscanf(str, "%u %u %u%n", &r, &g, &b, &end) == 3 && end >= len) {
		RET(r, g, b, 255);
	}

	return {};

#undef RET
}
