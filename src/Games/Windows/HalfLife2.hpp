#pragma once
#include "Game.hpp"

class HalfLife2 : public Game {
public:
    HalfLife2();
    void LoadOffsets();
    void LoadRules();
    const char* GetVersion();
};
