#pragma once
#include "Utils.hpp"

namespace Patterns {

std::vector<Memory::Pattern> Items;

void Create(const char* moduleName, const char* patternName)
{
    Items.push_back(Memory::Pattern{
        moduleName,
        patternName });
}
Memory::Pattern* Get(const char* patternName)
{
    for (auto& pattern : Items) {
        if (pattern.Name == patternName) {
            return &pattern;
        }
    }
    return nullptr;
}
void Add(const char* patternName, const char* version, const char* sigName, const char* sigBytes, const int offset = 0)
{
    Get(patternName)->SetSignature(version, sigName, sigBytes, offset);
}
void Inherit(const char* patternName, const char* version, const char* sigName)
{
    Get(patternName)->SetSignature(version, sigName);
}
void Init()
{
    Create(MODULE("engine"), "ConCommandCtor");
    Create(MODULE("engine"), "ConVarCtor");
    Create(MODULE("engine"), "m_bLoadgame");
    Create(MODULE("engine"), "Key_SetBinding");
    Create(MODULE("engine"), "AutoCompletionFunc");
    Create(MODULE("vguimatsurface"), "StartDrawing");
    Create(MODULE("vguimatsurface"), "FinishDrawing");
    Create(MODULE("server"), "FireOutput");
}
}