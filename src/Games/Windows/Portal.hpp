#pragma once
#include "HalfLife2.hpp"

class Portal : public HalfLife2 {
public:
    Portal();
    void LoadOffsets() override;
    const char* Version() override;

    static const char* ModDir();
};
