#pragma once
#include "Replay.hpp"

#include "Features/Feature.hpp"

#include "Command.hpp"
#include "Variable.hpp"

#define SAR_TAS_REPLAY_HEADER001 "sar-tas-replay v1.7"
#define SAR_TAS_REPLAY_HEADER002 "sar-tas-replay v1.8"
#define SAR_TAS_REPLAY_EXTENSION ".str"

class ReplaySystem : public Feature {
private:
    std::vector<Replay*> replays;
    int replayIndex;
    bool isRecording;
    bool isPlaying;

public:
    ReplaySystem();
    ~ReplaySystem();
    void Record(bool rerecord = false);
    void Play();
    void Stop();
    bool IsRecording();
    bool IsPlaying();
    Replay* GetCurrentReplay();
    bool AnyReplaysLoaded();
    void DeleteAll();
    void MergeAll();
    void MergeViews();

private:
    void NewReplay();
    void DeleteReplay();

public:
    void Export(std::string filePath, int index = 0);
    void Import(std::string filePath);
};

extern ReplaySystem* tasReplaySystem;

extern Variable sar_replay_autorecord;
extern Variable sar_replay_autoplay;

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
