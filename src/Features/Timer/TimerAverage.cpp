#include "TimerAverage.hpp"

TimerAverage::TimerAverage()
	: isEnabled(false)
	, items()
	, averageTicks(0)
	, averageTime(0) {
}
void TimerAverage::Start() {
	this->items.clear();
	this->averageTicks = 0;
	this->averageTime = 0;
	this->isEnabled = true;
}
void TimerAverage::Add(int ticks, float time, std::string map) {
	this->items.push_back(TimerAverageItem{
		ticks,
		time,
		map});
	auto count = this->items.size();
	this->averageTicks += ticks / count;
	this->averageTime += time / count;
}
