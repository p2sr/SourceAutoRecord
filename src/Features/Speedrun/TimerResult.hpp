#pragma once
#include <vector>

#include "TimerSplit.hpp"

class TimerResult {
public:
    int total;
    TimerSplit* curSplit;
    TimerSplit* prevSplit;
    std::vector<TimerSplit*> splits;

    TimerResult();
    void NewSplit(const int started, const char* map);
    void EndSplit(const int finished);
    void Split(const int ticks, const char* map);
    void AddSegment(int ticks);
    void UpdateSplit(const char* map);
    void Reset();
    ~TimerResult();
};
