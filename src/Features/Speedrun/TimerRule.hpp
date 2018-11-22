#pragma once
#include <vector>

#include "TimerAction.hpp"

#include "Game.hpp"

using _TimerRuleCallback0 = TimerAction (*)(void* entity);
using _TimerRuleCallback1 = TimerAction (*)(void* entity, int* prop);
using _TimerRuleCallback2 = TimerAction (*)(void* entity, int prop_offset);

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
        _TimerRuleCallback0 callback0;
        _TimerRuleCallback1 callback1;
        _TimerRuleCallback2 callback2;
    };

    void* entityPtr;
    int callbackType;
    int propOffset;
    bool isActive;

public:
    static std::vector<TimerRule*> list;

public:
    TimerRule(int gameVersion, const char* categoryName, const char* mapName, const char* entityName,
        _TimerRuleCallback0);
    TimerRule(int gameVersion, const char* categoryName, const char* mapName, const char* entityName,
        _TimerRuleCallback1, const char* className, const char* propName);
    TimerRule(int gameVersion, const char* categoryName, const char* mapName, const char* entityName,
        _TimerRuleCallback2, const char* className, const char* propName);

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
#define SAR_RULE2(gameName, categoryName, mapName, entityName, className, propName)                                                                                                                           \
    TimerAction gameName##_##categoryName##_##mapName##_callback(void* entityName, int propName##_offset);                                                                                                    \
    TimerRule gameName##_##categoryName##_##mapName##_rule = TimerRule(SourceGame_##gameName, #categoryName, #mapName, #entityName, gameName##_##categoryName##_##mapName##_callback, #className, #propName); \
    TimerAction gameName##_##categoryName##_##mapName##_callback(void* entityName, int propName##_offset)
#define SAR_RULE3(gameName, categoryName, mapName, entityName)                                                                                                                         \
    TimerAction gameName##_##categoryName##_##mapName##_callback(void* entityName);                                                                                                    \
    TimerRule gameName##_##categoryName##_##mapName##_rule = TimerRule(SourceGame_##gameName, #categoryName, #mapName, #entityName, gameName##_##categoryName##_##mapName##_callback); \
    TimerAction gameName##_##categoryName##_##mapName##_callback(void* entityName)
