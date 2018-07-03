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
    Create("engine.dll", "ConVar_Ctor3");
    Create("engine.dll", "ConCommand_Ctor1");
    Create("engine.dll", "ConCommand_Ctor2");
    Create("engine.dll", "m_bLoadgame");
    Create("engine.dll", "Key_SetBinding");
    Create("engine.dll", "AutoCompletionFunc");
    Create("vguimatsurface.dll", "StartDrawing");
    Create("vguimatsurface.dll", "FinishDrawing");
    Create("client.dll", "FindElement");
}
}