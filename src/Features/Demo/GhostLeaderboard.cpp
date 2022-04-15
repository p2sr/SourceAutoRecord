#include "GhostLeaderboard.hpp"
#include "NetworkGhostPlayer.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Event.hpp"

#define LB_PADDING 10
#define LB_COL_SPACING 10
#define ANIM_SPEED 4.0    // positions per second
#define MAX_ANIM_TIME 1.0 // seconds

Variable ghost_leaderboard_font("ghost_leaderboard_font", "68", 0, "The font to use for the ghost leaderboard.\n");
Variable ghost_leaderboard_max_display("ghost_leaderboard_max_display", "10", 0, "The maximum number of names to display on the leaderboard.\n");
Variable ghost_leaderboard_mode("ghost_leaderboard_mode", "0", 0, 1, "The mode for the leaderboard. 0 = CM, 1 = race\n");
Variable ghost_leaderboard_x("ghost_leaderboard_x", "10", 0, "The x position of the leaderboard.\n");
Variable ghost_leaderboard_y("ghost_leaderboard_y", "10", 0, "The x position of the leaderboard.\n");

static std::string formatTicks(int ticks) {
	if (ticks == INT_MAX) return "-";
	return SpeedrunTimer::Format((float)ticks / 60.0);
}

void GhostLeaderboardHud::Paint(int slot) {
	if (slot != 0) return;
	if (!this->active) return;

	unsigned nents = ghost_leaderboard_max_display.GetInt();
	if (nents > this->entries.size()) nents = this->entries.size();

	if (nents == 0) return;

	std::vector<LeaderboardEntry> lb;

	for (auto &ent : this->entries) {
		if (ent.display_position < (float)nents - 0.5) {
			lb.push_back(ent);
		}
	}

	// we've found all the relevant leaderboard entries, now calc size etc

	auto font = scheme->GetFontByID(ghost_leaderboard_font.GetInt());

	int rank_col_width = 0;
	int name_col_width = 0;
	int time_col_width = 0;

	for (auto &ent : lb) {
		int rank_width = surface->GetFontLength(font, "%d", ent.rank);
		int name_width = surface->GetFontLength(font, "%s", ent.name.c_str());
		int time_width = surface->GetFontLength(font, "%s", formatTicks(ent.ticks).c_str());
		if (rank_width > rank_col_width) rank_col_width = rank_width;
		if (name_width > name_col_width) name_col_width = name_width;
		if (time_width > time_col_width) time_col_width = time_width;
	}

	int ent_height = surface->GetFontHeight(font) + LB_PADDING;

	int height = LB_PADDING + nents*ent_height;
	int width = rank_col_width + name_col_width + time_col_width + 2*LB_PADDING + 2*LB_COL_SPACING;

	// finally start lol
	int x = ghost_leaderboard_x.GetInt();
	int y = ghost_leaderboard_y.GetInt();

	// background
	surface->DrawRect({0,0,0,192}, x, y, x+width, y+height);

	int rank_col_x = x + LB_PADDING;
	int name_col_x = x + LB_PADDING + rank_col_width + LB_COL_SPACING;
	int time_col_x = x + LB_PADDING + rank_col_width + LB_COL_SPACING + name_col_width + LB_COL_SPACING;

	for (auto &ent : lb) {
		int ent_y = y + LB_PADDING + ent.display_position*ent_height;

		Color col =
			ent.ticks == INT_MAX ? Color{255,255,255} :
			ent.rank == 1 ? Color{237,188,74} :
			ent.rank == 2 ? Color{181,191,201} :
			ent.rank == 3 ? Color{223,146,67} :
			Color{255,255,255};

		// draw rank
		int rank_width = surface->GetFontLength(font, "%d", ent.rank);
		int rank_x = rank_col_x + (rank_col_width-rank_width)/2;
		surface->DrawTxt(font, rank_x, ent_y, col, "%d", ent.rank);

		// draw name
		surface->DrawTxt(font, name_col_x, ent_y, col, "%s", ent.name.c_str());

		// draw time
		int time_width = surface->GetFontLength(font, "%s", formatTicks(ent.ticks).c_str());
		int time_x = time_col_x + (time_col_width-time_width)/2;
		surface->DrawTxt(font, time_x, ent_y, col, "%s", formatTicks(ent.ticks).c_str());
	}
}

void GhostLeaderboardHud::UpdateDisplayPositions() {
	auto now = NOW_STEADY();
	long ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->lastDisplayUpdate).count();
	this->lastDisplayUpdate = now;

	for (auto &ent : this->entries) {
		float tgt = ent.position;
		float cur = ent.display_position;

		float maxDelta = (float)ms / 1000.0 * ent.anim_speed;
		if (maxDelta < 0.001) maxDelta = 0.001; // 1k fps sorta moment

		if (fabsf(tgt - cur) < maxDelta) {
			ent.display_position = ent.position;
		} else if (tgt < cur) {
			ent.display_position -= maxDelta;
		} else {
			ent.display_position += maxDelta;
		}
	}
}

void GhostLeaderboardHud::UpdateLeaderboard() {
	// this is so bullshit lol
	struct EntryCompare {
		bool operator()(const LeaderboardEntry *a, const LeaderboardEntry *b) const {
			if (a->ticks == b->ticks) {
				// ensure consistent ordering
				return a->name < b->name;
			}
			return a->ticks < b->ticks;
		}
	};
	std::set<LeaderboardEntry *, EntryCompare> sorted;
	for (auto &ent : this->entries) sorted.insert(&ent);
	int last_time = -1;
	int set_rank = 0;
	int this_rank = 0;
	for (auto ent : sorted) {
		++this_rank;
		if (ent->ticks > last_time) {
			// okay, we doin worse, i like this
			set_rank = this_rank;
			last_time = ent->ticks;
		}
		int old_target = ent->position;
		ent->position = this_rank - 1;
		ent->rank = set_rank;

		if (old_target != ent->position) {
			// target moved - recalculate lerp speed
			float delta = fabsf((float)ent->position - ent->display_position);
			if (delta / ANIM_SPEED > MAX_ANIM_TIME) {
				// rescale speed to make it get to target in max time
				ent->anim_speed = delta / MAX_ANIM_TIME;
			} else {
				ent->anim_speed = ANIM_SPEED;
			}
		}
	}
}

void GhostLeaderboardHud::PurgeOld() {
	// clear old entries
	for (size_t i = 0; i < this->entries.size(); ++i) {
		auto &ent = this->entries[i];
		if (!networkManager.isConnected || (!networkManager.GetGhostByID(ent.ghost_id) && networkManager.ID != ent.ghost_id)) {
			// it's the purge or some shit
			this->entries.erase(this->entries.begin() + i);
			--i;
		}
	}
}

void GhostLeaderboardHud::AddNew(uint32_t id, const std::string &name) {
	// check we don't already have one with this id
	for (auto &ent : this->entries) {
		if (ent.ghost_id == id) {
			// update name ig, shouldn't happen unless we hit a dumb race condition
			ent.name = name;
			return;
		}
	}

	int pos = this->entries.size(); // add at the bottom so we don't show new people at the top for a frame
	this->entries.push_back(LeaderboardEntry{
		id,
		name,
		INT_MAX, // special value indicating no time yet. will be ranked worst but display as just -
		0,
		pos,
		(float)pos,
		ANIM_SPEED,
		true,
		false,
	});
}

LeaderboardEntry *GhostLeaderboardHud::GetEntryForGhost(uint32_t id) {
	for (auto &ent : this->entries) {
		if (ent.ghost_id == id) return &ent;
	}
	return nullptr;
}

void GhostLeaderboardHud::GhostLoad(uint32_t id, int ticks, bool sync) {
	auto ent = GetEntryForGhost(id);
	if (!ent) return;
	int mode = ghost_leaderboard_mode.GetInt();
	if (mode == 0) {
		// CM - don't give a fuck
		return;
	} else { // mode == 1
		// race
		if (!ent->finished) {
			ent->ticks = ticks;
			ent->waiting = sync;
		}
	}
}

void GhostLeaderboardHud::GhostFinished(uint32_t id, int ticks) {
	auto ent = GetEntryForGhost(id);
	if (!ent) return;
	int mode = ghost_leaderboard_mode.GetInt();
	if (mode == 0) {
		// CM
		if (ent->ticks > ticks) ent->ticks = ticks;
	} else { // mode == 1
		// race
		ent->ticks = ticks;
		ent->finished = true;
	}
}

void GhostLeaderboardHud::SpeedrunStart(int ticks) {
	int mode = ghost_leaderboard_mode.GetInt();
	if (mode == 0) {
		// CM - don't give a fuck
		return;
	} else { // mode == 1
		// race
		this->lastLiveUpdate = (int)(engine->GetHostTime() * 60.0f);
		for (auto &ent : ghostLeaderboard.entries) {
			ent.ticks = ticks;
			ent.waiting = false;
			ent.finished = false;
		}
	}
}

void GhostLeaderboardHud::UpdateLive() {
	if (ghost_leaderboard_mode.GetInt() != 1) return;

	int newTime = (int)(engine->GetHostTime() * 60.0f);
	int delta = newTime - this->lastLiveUpdate;
	this->lastLiveUpdate = newTime;

	if (delta < 0) return;

	for (auto &ent : ghostLeaderboard.entries) {
		if (ent.ticks != INT_MAX && !ent.finished && !ent.waiting) {
			ent.ticks += delta;
		}
	}
}

void GhostLeaderboardHud::UpdateSelf() {
	if (ghost_leaderboard_mode.GetInt() != 1) return;
	if (!SpeedrunTimer::IsRunning()) return;

	auto ent = GetEntryForGhost(networkManager.ID);
	if (!ent) return;

	// only do this update if we've deviated too far in the estimate -
	// otherwise rankings compared to other players are really fuckin
	// janky
	int ticks = SpeedrunTimer::GetTotalTicks();
	if (abs(ticks - ent->ticks) > 6) ent->ticks = ticks;
}

void GhostLeaderboardHud::SyncReady() {
	for (auto &ent : ghostLeaderboard.entries) {
		ent.waiting = false;
	}
}

GhostLeaderboardHud ghostLeaderboard;

ON_EVENT(FRAME) {
	ghostLeaderboard.PurgeOld();
	ghostLeaderboard.UpdateLive();
	ghostLeaderboard.UpdateSelf();
	ghostLeaderboard.UpdateLeaderboard();
	ghostLeaderboard.UpdateDisplayPositions();
}

Command ghost_leaderboard_on("+ghost_leaderboard", +[](const CCommand &args){ ghostLeaderboard.active = true; }, "+ghost_leaderboard - enable the ghost leaderboard HUD\n");
Command ghost_leaderboard_off("-ghost_leaderboard", +[](const CCommand &args){ ghostLeaderboard.active = false; }, "-ghost_leaderboard - disable the ghost leaderboard HUD\n");

CON_COMMAND(ghost_leaderboard_reset, "ghost_leaderboard_reset - reset all leaderboard entries to \"no time\".\n") {
	for (auto &ent : ghostLeaderboard.entries) {
		ent.ticks = INT_MAX;
		ent.finished = true;
		ent.waiting = false;
	}
}
