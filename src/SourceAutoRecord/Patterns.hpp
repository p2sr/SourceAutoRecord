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
    Create("engine.so", "ConVar_Ctor3");
    Create("engine.so", "ConCommand_Ctor1");
    Create("engine.so", "ConCommand_Ctor2");
    Create("engine.so", "m_bLoadgame");
    Create("engine.so", "Key_SetBinding");
    Create("engine.so", "AutoCompletionFunc");
    Create("vguimatsurface.so", "StartDrawing");
    Create("vguimatsurface.so", "FinishDrawing");
}
}