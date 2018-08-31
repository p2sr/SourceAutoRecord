#pragma once
#include "Feature.hpp"

struct SummaryItem {
    int ticks;
    float time;
    char* map;
};

class Summary : public Feature {
public:
    bool isRunning;
    std::vector<SummaryItem> items;
    int totalTicks;

public:
    Summary();
    void Start();
    void Add(int ticks, float time, char* map);
};

extern Summary* summary;
