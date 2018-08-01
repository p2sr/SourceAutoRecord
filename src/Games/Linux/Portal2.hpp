#pragma once
#include "Game.hpp"

class Portal2 : public Game {
public:
    Portal2();
    void LoadOffsets();
    void LoadRules();
    const char* GetVersion();
};
