#include "Summary.hpp"

Summary::Summary()
    : isRunning(false)
    , items()
    , totalTicks(0)
{
    this->hasLoaded = true;
}
void Summary::Start()
{
    this->items.clear();
    this->totalTicks = 0;
    this->isRunning = true;
}
void Summary::Add(int ticks, float time, char* map)
{
    this->items.push_back(SummaryItem{
        ticks,
        time,
        map });
    this->totalTicks += ticks;
}

Summary* summary;
