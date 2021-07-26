#pragma once
#include "Features/Feature.hpp"
#include "Replay.hpp"
#include "Utils.hpp"

class ReplayRecorder : public Feature {
private:
	int viewIndex;
	bool isRecording;

public:
	static int count;

public:
	ReplayRecorder();
	void Record(Replay *replay, CUserCmd *cmd);
	void StartRecording();
	void StopRecording();
	bool IsRecording();
	int GetViewId();
};

extern ReplayRecorder *replayRecorder1;
extern ReplayRecorder *replayRecorder2;
