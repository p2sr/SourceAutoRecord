#pragma once
#include <vector>

#include "TimerRule.hpp"

#include "Game.hpp"

class TimerCategory {
public:
    int gameVersion;
    const char* name;
    std::vector<TimerRule*> rules;

public:
    static std::vector<TimerCategory*> list;

public:
    TimerCategory(int gameVersion, const char* name, std::vector<TimerRule*> rules);

    static int FilterByGame(Game* game);
    static void ResetAll();
};

#define SAR_CATEGORY(gameVersion, name, rules) \
    TimerCategory gameVersion##_##name(SourceGame_##gameVersion, #name, std::vector<TimerRule*>(rules))
