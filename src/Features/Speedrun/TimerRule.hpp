#pragma once
#include "Utils/SDK.hpp"

#include "TimerAction.hpp"

class SpeedrunTimer;

class TimerRule {
public:
    const char* map;
    const char* target;
    const char* targetInput;
    TimerAction action;

public:
    TimerRule(const char* map, const char* target, const char* targetInput, TimerAction action);
    void Check(const EventQueuePrioritizedEvent_t* event, const int* engineTicks, SpeedrunTimer* timer);
};
