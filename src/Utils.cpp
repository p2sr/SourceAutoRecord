#include "Utils.hpp"

bool Utils::EndsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && !str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}
bool Utils::StartsWith(const char* str, const char* subStr)
{
    return std::strlen(str) >= std::strlen(subStr) && std::strstr(str, subStr) == str;
}
