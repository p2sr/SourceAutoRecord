#pragma once
#include "HalfLife2.hpp"

class HalfLife2Unpack : public HalfLife2 {
public:
    HalfLife2Unpack();
    void LoadOffsets() override;
    const char* Version() override;
    const float Tickrate() override;

    static const char* ModDir();
};
