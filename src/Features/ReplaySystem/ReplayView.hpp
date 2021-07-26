#pragma once
#include "ReplayFrame.hpp"
#include "Utils/SDK.hpp"

#include <vector>

class ReplayView {
public:
	std::vector<ReplayFrame> frames;
	int playIndex;

public:
	ReplayView();
	bool Ended();
	void Reset();
	void Resize();
};
