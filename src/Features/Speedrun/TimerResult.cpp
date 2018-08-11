#include "TimerResult.hpp"

#include <cstring>

#include "Modules/Console.hpp"

#include "TimerSplit.hpp"

TimerResult::TimerResult()
    : total(0)
    , curSplit(nullptr)
    , prevSplit(nullptr)
    , splits()
{
}
void TimerResult::NewSplit(const int started, const char* map)
{
    this->prevSplit = this->curSplit;
    this->curSplit = new TimerSplit(started, map);
}
void TimerResult::EndSplit(const int finished)
{
    this->curSplit->finished = finished;
    this->splits.push_back(this->curSplit);
}
void TimerResult::Split(const int ticks, const char* map)
{
    this->EndSplit(ticks);
    this->NewSplit(ticks, map);
}
void TimerResult::AddSegment(int ticks)
{
    this->curSplit->segments.push_back(TimerSegment { ticks });
}
void TimerResult::UpdateSplit(char* map)
{
    for (const auto& split : this->splits) {
        if (!std::strcmp(map, split->map)) {
            this->prevSplit = this->curSplit;
            this->curSplit = split;
        }
    }
}
void TimerResult::Reset()
{
    this->total = 0;
    this->prevSplit = nullptr;

    if (this->curSplit) {
        delete this->curSplit;
        this->curSplit = nullptr;
    }

    for (auto it = this->splits.begin(); it != this->splits.end();) {
        delete *it;
        it = this->splits.erase(it);
    }
}
TimerResult::~TimerResult()
{
    this->Reset();
}
