#include "TimerRule.hpp"

#include <cstring>

#include "SpeedrunTimer.hpp"
#include "TimerAction.hpp"

TimerRule::TimerRule(const char* map, const char* target, const char* targetInput, TimerAction action)
{
    this->map = map;
    this->target = target;
    this->targetInput = targetInput;
    this->action = action;
}
void TimerRule::Check(const EventQueuePrioritizedEvent_t* event, const int* engineTicks, SpeedrunTimer* timer)
{
    if (std::strcmp(this->map, timer->GetCurrentMap()) || !event)
        return;
    if (!event->m_iTarget || std::strcmp(this->target, event->m_iTarget))
        return;
    if (!event->m_iTargetInput || std::strcmp(this->targetInput, event->m_iTargetInput))
        return;
    if (this->action == TimerAction::Start)
        timer->Start(engineTicks);
    else if (this->action == TimerAction::End)
        timer->Stop();
}
