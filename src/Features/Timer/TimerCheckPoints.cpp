#include "TimerCheckPoints.hpp"

TimerCheckPoints::TimerCheckPoints()
	: items()
	, latestTick(0)
	, latestTime(0) {
}
void TimerCheckPoints::Add(int ticks, float time, std::string map) {
	this->items.push_back(TimerCheckPointItem{
		ticks,
		time,
		map});
	this->latestTick = ticks;
	this->latestTime = time;
}
void TimerCheckPoints::Reset() {
	this->items.clear();
	this->latestTick = 0;
	this->latestTime = 0;
}
