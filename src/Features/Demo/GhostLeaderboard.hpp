#include <chrono>
#include <set>
#include <string>
#include <vector>
#include "Features/Hud/Hud.hpp"
#include "GhostEntity.hpp"

struct LeaderboardEntry {
	uint32_t ghost_id;
	std::string name;
	int ticks;
	int rank; // 1-indexed
	int position; // 0-indexed
	float display_position; // 0-indexed
	float anim_speed;
	bool finished; // only used in race (not cm)
	bool waiting;
};

class GhostLeaderboardHud : public Hud {
public:
	GhostLeaderboardHud()
		: Hud(HudType_InGame | HudType_Paused, false)
		, lastLiveUpdate(-1)
	{ }

	virtual bool GetCurrentSize(int &w, int &h) override {
		return false;
	}

	virtual void Paint(int slot) override;

	void UpdateDisplayPositions();
	void UpdateLeaderboard();
	void PurgeOld();
	void AddNew(uint32_t id, const std::string &name);
	LeaderboardEntry *GetEntryForGhost(uint32_t id);
	void GhostLoad(uint32_t id, int ticks, bool sync);
	void GhostFinished(uint32_t id, int ticks);
	void SpeedrunStart(int ticks);
	void UpdateLive();
	void UpdateSelf();
	void SyncReady();

	bool active = false;
	int lastLiveUpdate;
	std::vector<LeaderboardEntry> entries;
	std::chrono::time_point<std::chrono::steady_clock> lastDisplayUpdate;
};

extern GhostLeaderboardHud ghostLeaderboard;
