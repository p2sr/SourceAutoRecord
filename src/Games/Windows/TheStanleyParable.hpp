#pragma once
#include "Portal2.hpp"

class TheStanleyParable : public Portal2 {
public:
    TheStanleyParable();
    void LoadOffsets();
    void LoadRules();
    const char* GetVersion();
};