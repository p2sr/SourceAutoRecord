#pragma once
#include "TheStanleyParable.hpp"

class TheBeginnersGuide : public TheStanleyParable {
public:
    TheBeginnersGuide();
    void LoadOffsets() override;
    void LoadRules() override;
    const char* Version() override;

    static const char* Process();
};
