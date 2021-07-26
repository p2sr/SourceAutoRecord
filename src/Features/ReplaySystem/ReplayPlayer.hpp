#pragma once
#include "Features/Feature.hpp"
#include "Replay.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

class ReplayPlayer : public Feature {
private:
	int viewIndex;
	bool isPlaying;

public:
	static int count;

public:
	ReplayPlayer();
	void Play(Replay *replay, CUserCmd *cmd);
	void StartPlaying(Replay *replay);
	void StopPlaying();
	bool IsPlaying();
};

extern ReplayPlayer *replayPlayer1;
extern ReplayPlayer *replayPlayer2;

extern Variable sar_replay_autoloop;
