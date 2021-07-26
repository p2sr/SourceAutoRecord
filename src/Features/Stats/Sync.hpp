#pragma once

#include "Command.hpp"
#include "Features/Feature.hpp"
#include "Variable.hpp"

#include <chrono>

class Sync : public Feature {
private:
	unsigned totalStrafeDelta[2];
	unsigned syncedStrafeDelta[2];
	float strafeSync[2];

	bool run;

	std::chrono::time_point<std::chrono::steady_clock> start;

public:
	std::vector<float> splits;

public:
	Sync();

	void UpdateSync(int slot, const CUserCmd *pMove);
	void PauseSyncSession();
	void ResumeSyncSession();
	void ResetSyncSession();
	void SplitSyncSession();

	float GetStrafeSync(int slot);
};

extern Sync *synchro;

extern Variable sar_strafesync;
extern Variable sar_strafesync_session_time;
extern Variable sar_strafesync_noground;
extern Command sar_strafesync_pause;
extern Command sar_strafesync_resume;
extern Command sar_strafesync_reset;
extern Command sar_strafesync_split;
