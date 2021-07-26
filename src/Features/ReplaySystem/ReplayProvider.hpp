#pragma once
#include "Command.hpp"
#include "Features/Feature.hpp"
#include "Replay.hpp"
#include "Variable.hpp"

#define SAR_TAS_REPLAY_HEADER001 "sar-tas-replay v1.7"
#define SAR_TAS_REPLAY_HEADER002 "sar-tas-replay v1.8"
#define SAR_TAS_REPLAY_EXTENSION ".str"

class ReplayProvider : public Feature {
public:
	std::vector<Replay *> replays;

public:
	ReplayProvider();
	~ReplayProvider();
	void CreateNewReplay();
	Replay *GetCurrentReplay();
	bool AnyReplaysLoaded();
	void DeleteAll();
	void MergeAll();
	void MergeViews(int firstReplay, int secondReplay, int firstView, int secondView);

public:
	void Export(const char *fileName, int index = 0);
	void Import(const char *fileName);
};

extern ReplayProvider *replayProvider;

extern Variable sar_replay_mode;
extern Variable sar_replay_viewmode;

extern Command sar_replay_record;
extern Command sar_replay_record_again;
extern Command sar_replay_play;
extern Command sar_replay_stop;
extern Command sar_replay_merge_all;
extern Command sar_replay_merge_views;
extern Command sar_replay_export;
extern Command sar_replay_export_at;
extern Command sar_replay_import;
extern Command sar_replay_import2;
