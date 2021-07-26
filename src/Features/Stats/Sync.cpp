#include "Sync.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Utils/SDK.hpp"

#include <algorithm>

Variable sar_strafesync("sar_strafesync", "0", "Shows strafe sync stats.\n");
Variable sar_strafesync_session_time("sar_strafesync_session_time", "0", 0,
                                     "In seconds. How much time should pass until session is reset.\n"
                                     "If 0, you'll have to reset the session manually.\n");
Variable sar_strafesync_noground("sar_strafesync_noground", "1",
                                 "0: Always run.\n"
                                 "1: Do not run when on ground.\n");

Sync *synchro;

Sync::Sync()
	: totalStrafeDelta{0, 0}
	, syncedStrafeDelta{0, 0}
	, strafeSync{100, 100}
	, run(true) {
	this->hasLoaded = true;
	this->start = std::chrono::steady_clock::now();
}

void Sync::UpdateSync(int slot, const CUserCmd *cmd) {
	if (!this->run) {
		return;
	}

	auto player = client->GetPlayer(slot + 1);

	if (sar_strafesync_noground.GetBool()) {
		unsigned int groundEntity = *reinterpret_cast<unsigned int *>((uintptr_t)player + Offsets::C_m_hGroundEntity);
		bool grounded = groundEntity != 0xFFFFFFFF;
		if (grounded) {
			return;
		}
	}

	if (sar_strafesync_session_time.GetInt() > 0) {
		if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - this->start).count() >= sar_strafesync_session_time.GetInt()) {
			this->ResetSyncSession();
			this->start = std::chrono::steady_clock::now();
		}
	}

	bool mvLeft = cmd->buttons & IN_MOVELEFT;
	bool mvRight = cmd->buttons & IN_MOVERIGHT;

	int dtAngle = cmd->mousedx;

	if (dtAngle < 0 && (mvLeft ^ mvRight)) {  //Player turned left
		this->totalStrafeDelta[slot] -= dtAngle;
		if (mvLeft && !mvRight) {
			this->syncedStrafeDelta[slot] -= dtAngle;
		}
	} else if (dtAngle > 0 && (mvLeft ^ mvRight)) {  //Player turned right
		this->totalStrafeDelta[slot] += dtAngle;
		if (mvRight && !mvLeft) {
			this->syncedStrafeDelta[slot] += dtAngle;
		}
	}

	if (this->totalStrafeDelta[slot]) {
		this->strafeSync[slot] = ((float)this->syncedStrafeDelta[slot] / (float)this->totalStrafeDelta[slot]) * 100.0f;
	}
}

void Sync::PauseSyncSession() {
	this->run = false;
}

void Sync::ResumeSyncSession() {
	this->run = true;
}

void Sync::ResetSyncSession() {
	this->syncedStrafeDelta[0] = this->syncedStrafeDelta[1] = 0;
	this->totalStrafeDelta[0] = this->totalStrafeDelta[1] = 0;
	this->strafeSync[0] = this->strafeSync[1] = 100;
	this->splits.clear();
}

void Sync::SplitSyncSession() {
	this->splits.push_back(this->strafeSync[0]);
}

float Sync::GetStrafeSync(int slot) {
	return std::clamp(this->strafeSync[slot], 0.0f, 100.0f);
}

// Commands

CON_COMMAND(sar_strafesync_pause, "sar_strafesync_pause - pause strafe sync session\n") {
	synchro->PauseSyncSession();
}

CON_COMMAND(sar_strafesync_resume, "sar_strafesync_resume - resume strafe sync session\n") {
	synchro->ResumeSyncSession();
}

CON_COMMAND(sar_strafesync_reset, "sar_strafesync_reset - reset strafe sync session\n") {
	synchro->ResetSyncSession();
}

CON_COMMAND(sar_strafesync_split, "sar_strafesync_split - makes a new split\n") {
	synchro->SplitSyncSession();
}
