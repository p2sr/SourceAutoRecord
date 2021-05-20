#include "Sync.hpp"

#include "Utils/SDK.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include <algorithm>

Variable sar_strafesync("sar_strafesync", "0", "Shows strafe sync stats.\n");
Variable sar_strafesync_session_time("sar_strafesync_session_time", "0", 0, "In seconds. How much time should pass until session is reset.\n"
                                                                            "If 0, you'll have to reset the session manually.\n");
Variable sar_strafesync_noground("sar_strafesync_noground", "0", "0: Always run.\n"
                                                                 "1: Do not run when on ground.\n");

Sync* synchro;

Sync::Sync()
    : lastButtons(0)
    , oldAngles({ 0, 0, 0 })
    , strafeTick(0)
    , perfectSyncTick(0)
    , accelTicks(0)
    , strafeSync(100)
    , run(true)
{
    this->hasLoaded = true;
    this->start = std::chrono::steady_clock::now();
}

void Sync::SetLastDatas(const int buttons, const QAngle& ang)
{
    this->lastButtons = buttons;
    this->oldAngles = ang;
}

void Sync::UpdateSync(const CUserCmd* cmd)
{
    if (!this->run) {
        return;
    }

    auto player = server->GetPlayer(GET_SLOT() + 1);
        if (!player) {
            return;
    }

    auto currentAngles = server->GetAbsAngles(player);

    if (sar_strafesync_noground.GetBool()) {
        unsigned int groundEntity = *reinterpret_cast<unsigned int*>((uintptr_t)player + Offsets::S_m_hGroundEntity);
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

    float dtAngle = currentAngles.y - this->oldAngles.y;
    if (dtAngle > 180.f) {
        dtAngle -= 360.f;
    } else if (dtAngle < -180.f) {
        dtAngle += 360.f;
    }

    if (dtAngle > 0 && (mvLeft ^ mvRight)) { //Player turned left
        ++this->strafeTick;
        if (mvLeft && !mvRight) {
            ++this->perfectSyncTick;
        }
        if (cmd->sidemove < 0) {
            ++this->accelTicks;
        }
    } else if (dtAngle < 0 && (mvLeft ^ mvRight)) { //Player turned right
        ++this->strafeTick;
        if (mvRight && !mvLeft) {
            ++this->perfectSyncTick;
        }
        if (cmd->sidemove > 0) {
            ++this->accelTicks;
        }
    }

    if (this->strafeTick && this->accelTicks && this->perfectSyncTick) {
        this->strafeSync = (float(this->perfectSyncTick) / float(this->strafeTick)) * 100.0f;
    }

    this->SetLastDatas(cmd->buttons, currentAngles);
}

void Sync::PauseSyncSession()
{
    this->run = false;
}

void Sync::ResumeSyncSession()
{
    this->run = true;
}

void Sync::ResetSyncSession()
{
    this->perfectSyncTick = 0;
    this->strafeTick = 0;
    this->strafeSync = 100;
    this->splits.clear();
}

void Sync::SplitSyncSession()
{
    this->splits.push_back(this->strafeSync);
}

float Sync::GetStrafeSync()
{
    return std::clamp(this->strafeSync, 0.0f, 100.0f);
}

// Commands

CON_COMMAND(sar_strafesync_pause, "Pause strafe sync session.\n")
{
    synchro->PauseSyncSession();
}

CON_COMMAND(sar_strafesync_resume, "Resume strafe sync session.\n")
{
    synchro->ResumeSyncSession();
}

CON_COMMAND(sar_strafesync_reset, "Reset strafe sync session.\n")
{
    synchro->ResetSyncSession();
}

CON_COMMAND(sar_strafesync_split, "Makes a new split.\n")
{
    synchro->SplitSyncSession();
}
