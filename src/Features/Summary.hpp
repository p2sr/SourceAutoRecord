#pragma once
#include "Feature.hpp"
#include "Variable.hpp"

struct SummaryItem {
	int ticks;
	float time;
	std::string map;
};

class Summary : public Feature {
public:
	bool isRunning;
	std::vector<SummaryItem> items;
	int totalTicks;

public:
	Summary();
	void Start();
	void Add(int ticks, float time, std::string map);
};

extern Summary *summary;

extern Variable sar_sum_during_session;
