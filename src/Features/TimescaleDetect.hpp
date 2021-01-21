#pragma once

#include "Feature.hpp"

#include <chrono>

class TimescaleDetect : public Feature {
public:
    TimescaleDetect();
    void Update();
    void Cancel();
    void Spawn();
private:
    int spawnTick;
    int startTick;
    std::chrono::time_point<std::chrono::steady_clock> startTickTime;
};

extern TimescaleDetect* timescaleDetect;
