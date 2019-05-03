#pragma once
#include "Portal2.hpp"

class TheStanleyParable : public Portal2 {
public:
    TheStanleyParable();
    void LoadOffsets() override;
    const char* Version() override;

    static const char* ModDir();
};
