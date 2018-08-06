#pragma once
#include "Utils/SDK.hpp"

#include "TimerAction.hpp"

class SpeedrunTimer;

class TimerInterface {
public:
    char start[16]; // 0
    int total; // 16
    float ipt; // 20
    TimerAction action; // 24
    char end[14]; // 28

public:
    TimerInterface();
    void SetIntervalPerTick(const float* ipt);
    void Update(SpeedrunTimer* timer);
    void SetAction(TimerAction action);
};
