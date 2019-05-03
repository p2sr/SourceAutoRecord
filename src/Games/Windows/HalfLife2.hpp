#pragma once
#include "Game.hpp"

class HalfLife2 : public Game {
public:
    HalfLife2();
    void LoadOffsets() override;
    const char* Version() override;
    const float Tickrate() override;

    static const char* ModDir();
};
