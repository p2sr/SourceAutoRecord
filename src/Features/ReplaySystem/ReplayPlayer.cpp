#include "ReplayPlayer.hpp"

#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Modules/Server.hpp"
#include "Replay.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

Variable sar_replay_autoloop("sar_replay_autoloop", "0", "Plays replay again when it ended.\n");

ReplayPlayer *replayPlayer1;
ReplayPlayer *replayPlayer2;

ReplayPlayer::ReplayPlayer()
	: viewIndex(ReplayPlayer::count++)
	, isPlaying(false) {
}
void ReplayPlayer::Play(Replay *replay, CUserCmd *cmd) {
	auto view = replay->GetView(this->viewIndex);

	if (view->Ended()) {
		if (sar_replay_autoloop.GetBool()) {
			view->Reset();
		} else {
			this->isPlaying = false;
		}
	} else {
		auto frame = view->frames[view->playIndex];

		cmd->viewangles = frame.viewangles;
		cmd->forwardmove = frame.forwardmove;
		cmd->sidemove = frame.sidemove;
		cmd->upmove = frame.upmove;
		cmd->buttons = frame.buttons;
		cmd->impulse = frame.impulse;
		cmd->mousedx = frame.mousedx;
		cmd->mousedy = frame.mousedy;

		++view->playIndex;
	}
}
void ReplayPlayer::StartPlaying(Replay *replay) {
	if ((!SpeedrunTimer::IsRunning() && !sv_bonus_challenge.GetBool()) || sv_cheats.GetBool()) {
		replay->GetView(this->viewIndex)->Reset();
		this->isPlaying = true;
	}
}
void ReplayPlayer::StopPlaying() {
	this->isPlaying = false;
}
bool ReplayPlayer::IsPlaying() {
	return this->isPlaying;
}

int ReplayPlayer::count;
