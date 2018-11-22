#include "TimerInterface.hpp"

#include "SpeedrunTimer.hpp"
#include "TimerAction.hpp"

TimerInterface::TimerInterface()
    : start("SAR_TIMER_START")
    , total(0)
    , ipt(0)
    , action(TimerAction::DoNothing)
    , end("SAR_TIMER_END")
{
}
void TimerInterface::SetIntervalPerTick(const float* ipt)
{
    this->ipt = *ipt;
}
void TimerInterface::Update(SpeedrunTimer* timer)
{
    this->total = timer->GetTotal();
}
void TimerInterface::SetAction(TimerAction action)
{
    this->action = action;
}
