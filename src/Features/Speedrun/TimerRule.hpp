#pragma once
#include <stdint.h>
#include <vector>

#include "TimerAction.hpp"

#include "Game.hpp"

using _TimerRuleCallback = TimerAction (*)(void* entity, int* prop);
using _TimerRuleCallback2 = TimerAction (*)(void* entity);

class TimerRule {
public:
    bool madeAction;
    int gameVersion;
    const char* categoryName;
    const char* mapName;

private:    
    const char* entityName;
    const char* className;
    const char* propName;

    union {
        _TimerRuleCallback callback;
        _TimerRuleCallback2 callback2;
    };

    void* entityPtr;
    bool hasProps;
    int propOffset;
    bool isActive;

public:
    static std::vector<TimerRule*> list;

public:
    TimerRule(int gameVersion, const char* categoryName, const char* mapName, const char* entityName,
        _TimerRuleCallback2);
    TimerRule(int gameVersion, const char* categoryName, const char* mapName, const char* entityName,
        _TimerRuleCallback, const char* className, const char* propName);

    bool Load();
    void Unload();
    TimerAction Dispatch();

    static int FilterByGame(Game* game);
    static void ResetAll();
};

#define SAR_RULE(gameName, categoryName, mapName, entityName, className, propName)                                                                                                                            \
    TimerAction gameName##_##categoryName##_##mapName##_callback(void* entityName, int* propName);                                                                                                            \
    TimerRule gameName##_##categoryName##_##mapName##_rule = TimerRule(SourceGame_##gameName, #categoryName, #mapName, #entityName, gameName##_##categoryName##_##mapName##_callback, #className, #propName); \
    TimerAction gameName##_##categoryName##_##mapName##_callback(void* entityName, int* propName)
#define SAR_RULE2(gameName, categoryName, mapName, entityName)                                                                                                                         \
    TimerAction gameName##_##categoryName##_##mapName##_callback(void* entityName);                                                                                                    \
    TimerRule gameName##_##categoryName##_##mapName##_rule = TimerRule(SourceGame_##gameName, #categoryName, #mapName, #entityName, gameName##_##categoryName##_##mapName##_callback); \
    TimerAction gameName##_##categoryName##_##mapName##_callback(void* entityName)
