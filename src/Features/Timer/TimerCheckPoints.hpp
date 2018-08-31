#pragma once
#include <vector>

struct TimerCheckPointItem {
    int ticks;
    float time;
    char* map;
};

class TimerCheckPoints {
public:
    std::vector<TimerCheckPointItem> items;
    int latestTick;
    float latestTime;

public:
    TimerCheckPoints();
    void Add(int ticks, float time, char* map);
    void Reset();
};
