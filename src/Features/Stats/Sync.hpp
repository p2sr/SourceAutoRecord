#pragma once

#include "Command.hpp"
#include "Variable.hpp"

#include "Features/Feature.hpp"

#include <chrono>

class Sync : public Feature {

private:
    int lastButtons;
    QAngle oldAngles;

    int strafeTick;
    int perfectSyncTick;
    int accelTicks;
    float strafeSync;

    bool run;

    std::chrono::time_point<std::chrono::steady_clock> start;

public:
    std::vector<float> splits;

public:
    Sync();
    void SetLastDatas(const int buttons, const QAngle& ang);

    void UpdateSync(const CUserCmd* pMove);
    void PauseSyncSession();
    void ResumeSyncSession();
    void ResetSyncSession();
    void SplitSyncSession();

    float GetStrafeSync();
};

extern Sync* sync;

extern Variable sar_strafesync;
extern Variable sar_strafesync_session_time;
extern Variable sar_strafesync_noground;
extern Command sar_strafesync_pause;
extern Command sar_strafesync_resume;
extern Command sar_strafesync_reset;
extern Command sar_strafesync_split;
