#include "Utils.hpp"

#include <cctype>
#include <cstring>
#include <string>
#include <cstdarg>

bool Utils::EndsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && !str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}
bool Utils::StartsWith(const char* str, const char* subStr)
{
    return std::strlen(str) >= std::strlen(subStr) && std::strstr(str, subStr) == str;
}
bool Utils::ICompare(const std::string& a, const std::string& b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) {
        return std::tolower(a) == std::tolower(b);
    });
};
std::string Utils::ssprintf(const char *fmt, ...)
{
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
int Utils::ConvertFromSrgb(int s)
{
    double s_ = (double)s / 255;
    double l = s <= 0.04045 ? s_ / 12.92 : pow((s_ + 0.055) / 1.055, 2.4);
    return (int)(l * 255);
}
