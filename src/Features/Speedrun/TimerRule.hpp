#pragma once
#include "TimerAction.hpp"

#include "Utils/SDK.hpp"

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
