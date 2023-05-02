#include "Utils.hpp"

#include <cctype>
#include <cmath>
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
bool Utils::StartsWithInsens(const char *str, const char *subStr) {
	for (size_t i = 0; subStr[i]; ++i) {
		if (!str[i]) return false;
		char c1 = subStr[i];
		if (c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
		char c2 = str[i];
		if (c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';
		if (c1 != c2) return false;
	}
	return true;
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
uint8_t Utils::ConvertFromSrgb(uint8_t s) {
	double s_ = (double)s / 255;
	double l = s_ <= 0.04045 ? s_ / 12.92 : pow((s_ + 0.055) / 1.055, 2.4);
	return (uint8_t)(l * 255);
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
	return to_linear ? \
		Color{ConvertFromSrgb((uint8_t)r), ConvertFromSrgb((uint8_t)g), ConvertFromSrgb((uint8_t)b), (uint8_t)a} : \
		Color { (uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a }

	while (isspace(*str)) ++str;
	size_t len = strlen(str);
	for (size_t i = len - 1; i; --i) {
		if (!isspace(str[i])) break;
		--len;
	}

	bool had_hash = str[0] == '#';
	if (had_hash) ++str, --len;

	unsigned r, g, b, a;
	int end;

	if (len == 8 && sscanf(str, "%2x%2x%2x%2x%n", &r, &g, &b, &a, &end) == 4 && end >= 8) {
		RET(r, g, b, a);
	}

	if (len == 6 && sscanf(str, "%2x%2x%2x%n", &r, &g, &b, &end) == 3 && end >= 6) {
		RET(r, g, b, 255);
	}

	if (had_hash) return {};

	if (sscanf(str, "%u %u %u %u%n", &r, &g, &b, &a, &end) == 4 && (size_t)end >= len) {
		RET(r, g, b, a);
	}

	if (sscanf(str, "%u %u %u%n", &r, &g, &b, &end) == 3 && (size_t)end >= len) {
		RET(r, g, b, 255);
	}

	return {};

#undef RET
}
Color Utils::HSVToRGB(float H, float S, float V) {
	float s = S / 100;
	float v = V / 100;
	float C = s * v;
	float X = C * (1 - fabsf(fmod(H / 60.0, 2) - 1));
	float m = v - C;
	float r, g, b;

	if (H >= 0 && H < 60)
		r = C, g = X, b = 0;
	else if (H >= 60 && H < 120)
		r = X, g = C, b = 0;
	else if (H >= 120 && H < 180)
		r = 0, g = C, b = X;
	else if (H >= 180 && H < 240)
		r = 0, g = X, b = C;
	else if (H >= 240 && H < 300)
		r = X, g = 0, b = C;
	else
		r = C, g = 0, b = X;

	int R = (r + m) * 255;
	int G = (g + m) * 255;
	int B = (b + m) * 255;

	return Color(R, G, B);
}
const char *Utils::ArgContinuation(const CCommand &args, int from) {
	const char *text;

	if (args.ArgC() == from + 1) {
		text = args[from];
	} else {
		text = args.m_pArgSBuffer + args.m_nArgv0Size;
		if (from > 1) while (isspace(*text)) ++text;
		for (int i = 1; i < from; i++) {
			text += (*text == '"') * 2 + strlen(args[i]);
			while (isspace(*text)) ++text;
		}
	}

	return text;
}
