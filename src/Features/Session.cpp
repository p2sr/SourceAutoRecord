#include "Session.hpp"

#include "Features/Rebinder.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Stats/Stats.hpp"
#include "Features/StepCounter.hpp"
#include "Features/Summary.hpp"
#include "Features/Tas/CommandQueuer.hpp"
#include "Features/Tas/ReplaySystem.hpp"
#include "Features/Timer/Timer.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

Session* session;

Session::Session()
    : baseTick(0)
    , lastSession(0)
    , isInSession(false)
    , currentFrame(0)
    , lastFrame(0)
    , prevState(HOSTSTATES::HS_RUN)
{
    this->hasLoaded = true;
}
void Session::Rebase(const int from)
{
    this->baseTick = from;
}
void Session::Started(bool menu)
{
    if (this->isInSession) {
        return;
    }

    if (menu) {
        console->Print("Session started! (menu)\n");
        session->Rebase(*engine->tickcount);

        if (sar_speedrun_autostop.GetBool()) {
            speedrun->Stop(false);
        } else {
            speedrun->Unpause(engine->tickcount);
        }
    } else {
        if (engine->GetMaxClients() <= 1) {
            console->Print("Session Started!\n");
            session->Rebase(*engine->tickcount);
            timer->Rebase(*engine->tickcount);

            speedrun->Unpause(engine->tickcount);
        }

        if (rebinder->isSaveBinding || rebinder->isReloadBinding) {
            if (engine->demorecorder->isRecordingDemo) {
                rebinder->UpdateIndex(*engine->demorecorder->m_nDemoNumber);
            } else {
                rebinder->UpdateIndex(rebinder->lastIndexNumber + 1);
            }

            rebinder->RebindSave();
            rebinder->RebindReload();
        }

        if (sar_tas_autostart.GetBool()) {
            tasQueuer->Start();
        }
        if (sar_replay_autorecord.GetBool()) {
            tasReplaySystem->Record();
        }
        if (sar_replay_autoplay.GetBool()) {
            tasReplaySystem->Play();
        }
        if (sar_speedrun_autostart.GetBool() && !speedrun->IsActive()) {
            speedrun->Start(engine->tickcount);
        }

        stepCounter->ResetTimer();
        currentFrame = 0;
    }

    speedrun->ReloadRules();
    isInSession = true;
}
void Session::Ended()
{
    if (!this->isInSession) {
        return;
    }

    int tick = engine->GetSessionTick();

    if (tick != 0) {
        console->Print("Session: %i (%.3f)\n", tick, engine->ToTime(tick));
        session->lastSession = tick;
    }

    if (summary->isRunning) {
        summary->Add(tick, engine->ToTime(tick), engine->m_szLevelName);
        console->Print("Total: %i (%.3f)\n", summary->totalTicks, engine->ToTime(summary->totalTicks));
    }

    if (timer->isRunning) {
        if (sar_timer_always_running.GetBool()) {
            timer->Save(*engine->tickcount);
            console->Print("Timer paused: %i (%.3f)!\n", timer->totalTicks, engine->ToTime(timer->totalTicks));
        } else {
            timer->Stop(*engine->tickcount);
            console->Print("Timer stopped!\n");
        }
    }

    auto reset = sar_stats_auto_reset.GetInt();
    if ((reset == 1 && !*engine->m_bLoadgame) || reset >= 2) {
        stats->ResetAll();
    }

    engine->demorecorder->currentDemo = "";
    this->lastFrame = this->currentFrame;
    this->currentFrame = 0;

    tasQueuer->Stop();
    tasReplaySystem->Stop();
    speedrun->Pause();
    speedrun->UnloadRules();

    this->isInSession = false;
}
void Session::Changed()
{
    console->DevMsg("m_currentState = %i\n", engine->hoststate->m_currentState);

    if (engine->hoststate->m_currentState == HOSTSTATES::HS_CHANGE_LEVEL_SP
        || engine->hoststate->m_currentState == HOSTSTATES::HS_CHANGE_LEVEL_MP
        || engine->hoststate->m_currentState == HOSTSTATES::HS_GAME_SHUTDOWN) {
        this->Ended();
    } else if (engine->hoststate->m_currentState == HOSTSTATES::HS_RUN
        && !engine->hoststate->m_activeGame) {
        this->Started(true);
    }
}
void Session::Changed(int state)
{
    console->DevMsg("state = %i\n", state);

    // Demo recorder starts syncing from this tick
    if (state == SignonState::Full) {
        this->Started();
    } else {
        this->Ended();
    }
}
