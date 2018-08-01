#pragma once
#include "TheStanleyParable.hpp"

class TheBeginnersGuide : public TheStanleyParable {
public:
    TheBeginnersGuide();
    void LoadOffsets();
    void LoadRules();
    const char* GetVersion();
};
