#pragma once
#include "Game.hpp"

class Portal2 : public Game {
public:
    Portal2();
    void LoadOffsets() override;
    void LoadRules() override;
    const char* Version() override;
};
