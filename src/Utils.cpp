#include "Utils.hpp"

#include <cctype>
#include <cstring>
#include <string>

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
