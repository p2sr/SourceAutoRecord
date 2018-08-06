#pragma once
#include <vector>

class TimerSegment {
public:
    int session;
};

class TimerSplit {
public:
    int entered;
    int finished;
    char map[64];
    std::vector<TimerSegment> segments;

    TimerSplit(const int start, const char* map);
    int GetTotal();
};
