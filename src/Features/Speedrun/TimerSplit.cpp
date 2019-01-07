#include "TimerSplit.hpp"

#include <cstring>
#include <vector>

TimerSplit::TimerSplit(const int ticks, const char* map)
{
    this->entered = ticks;
    std::strncpy(this->map, map, sizeof(this->map));
    this->segments = std::vector<TimerSegment>();
}
int TimerSplit::GetTotal()
{
    auto total = this->finished - this->entered;
    if (total < 0) {
        total = 0;
        for (const auto& segment : this->segments) {
            total += segment.session;
        }
    }
    return total;
}
