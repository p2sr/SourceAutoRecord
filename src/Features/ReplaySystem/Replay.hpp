#pragma once
#include "ReplayView.hpp"
#include "Utils.hpp"

#include <vector>

class Replay {
private:
	std::vector<ReplayView> views;
	char source[MAX_PATH];

public:
	Replay(int viewSize);
	Replay(int viewSize, const char *source);
	ReplayView *GetView(int view);
	int GetViewSize();
	int GetFrameSize();
	const char *GetSource();
};
