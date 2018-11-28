#pragma once
#include <vector>

#include "TimerAction.hpp"

using _TimerRuleCallback0 = TimerAction (*)(void* entity);
using _TimerRuleCallback1 = TimerAction (*)(void* entity, int* prop);
using _TimerRuleCallback2 = TimerAction (*)(void* entity, int prop_offset);

enum class SearchMode {
    Classes,
    Names
};

class TimerRule {
public:
    bool madeAction;
    const char* name;
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
    SearchMode searchMode;

public:
    TimerRule(const char* name, const char* mapName, const char* entityName,
        _TimerRuleCallback0, SearchMode searchMode = SearchMode::Names);
    TimerRule(const char* name, const char* mapName, const char* entityName,
        _TimerRuleCallback1, const char* className, const char* propName, SearchMode searchMode = SearchMode::Names);
    TimerRule(const char* name, const char* mapName, const char* entityName,
        _TimerRuleCallback2, const char* className, const char* propName, SearchMode searchMode = SearchMode::Names);

    bool Load();
    void Unload();
    TimerAction Dispatch();
};

#define SAR_RULE(ruleName, mapName, entityName, className, propName, searchMode)                                              \
    TimerAction ruleName##_callback(void* entityName, int* propName);                                                         \
    TimerRule ruleName = TimerRule(#ruleName, #mapName, #entityName, ruleName##_callback, #className, #propName, searchMode); \
    TimerAction ruleName##_callback(void* entityName, int* propName)
#define SAR_RULE2(ruleName, mapName, entityName, className, propName, searchMode)                                             \
    TimerAction ruleName##_callback(void* entityName, int propName##_offset);                                                 \
    TimerRule ruleName = TimerRule(#ruleName, #mapName, #entityName, ruleName##_callback, #className, #propName, searchMode); \
    TimerAction ruleName##_callback(void* entityName, int propName##_offset)
#define SAR_RULE3(ruleName, mapName, entityName, searchMode)                                           \
    TimerAction ruleName##_callback(void* entityName);                                                 \
    TimerRule ruleName = TimerRule(#ruleName, #mapName, #entityName, ruleName##_callback, searchMode); \
    TimerAction ruleName##_callback(void* entityName)
