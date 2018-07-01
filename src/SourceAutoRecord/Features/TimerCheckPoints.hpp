#pragma once

namespace Timer {

namespace CheckPoints {
    struct CheckPointItem {
        int Ticks;
        float Time;
        char* Map;
    };

    std::vector<CheckPointItem> Items;

    int LatestTick;
    float LatestTime;

    void Add(int ticks, float time, char* map)
    {
        Items.push_back(CheckPointItem{
            ticks,
            time,
            map });
        LatestTick = ticks;
        LatestTime = time;
    }
    void Reset()
    {
        Items.clear();
        LatestTick = 0;
        LatestTime = 0;
    }
}
}