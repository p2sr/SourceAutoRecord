#include "Session.hpp"

#include "Features/Listener.hpp"
#include "Features/Rebinder.hpp"
#include "Features/ReplaySystem/ReplayPlayer.hpp"
#include "Features/ReplaySystem/ReplayProvider.hpp"
#include "Features/ReplaySystem/ReplayRecorder.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Stats/Stats.hpp"
#include "Features/StepCounter.hpp"
#include "Features/Summary.hpp"
#include "Features/Tas/CommandQueuer.hpp"
#include "Features/Timer/Timer.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Utils/SDK.hpp"

Session* session;

Session::Session()
    : baseTick(0)
    , lastSession(0)
    , isRunning(true)
    , currentFrame(0)
    , lastFrame(0)
    , prevState(HS_RUN)
{
    this->hasLoaded = true;
}
void Session::Rebase(const int from)
{
    this->baseTick = from;
}
void Session::Started(bool menu)
{
    if (this->isRunning) {
        return;
    }

    if (menu) {
        console->Print("Session started! (menu)\n");
        this->Rebase(*engine->tickcount);

        if (sar_speedrun_autostop.isRegistered && sar_speedrun_autostop.GetBool()) {
            speedrun->Stop(false);
        } else {
            speedrun->Resume(engine->tickcount);
        }

        this->isRunning = true;
    } else {
        console->Print("Session Started!\n");
        this->Start();
    }
}
void Session::Start()
{
    if (this->isRunning) {
        return;
    }

    this->Rebase(*engine->tickcount);
    timer->Rebase(*engine->tickcount);
    speedrun->Resume(engine->tickcount);

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
        cmdQueuer->Start();
    }

    if (sar_replay_mode.GetBool()) {
        if (sar_replay_mode.GetInt() == 1) {
            replayProvider->CreateNewReplay();
            replayRecorder1->StartRecording();

            if (replayProvider->GetCurrentReplay()->GetViewSize() > 1) {
                replayRecorder2->StartRecording();
            }
        } else if (replayProvider->AnyReplaysLoaded()) {
            auto replay = replayProvider->GetCurrentReplay();
            replayPlayer1->StartPlaying(replay);

            if (engine->GetMaxClients() > 1 && replay->GetViewSize() > 1) {
                replayPlayer2->StartPlaying(replay);
            }
        }
    } else if (sar_replay_viewmode.isRegistered && sar_replay_viewmode.GetBool() && replayProvider->AnyReplaysLoaded()) {
        auto replay = replayProvider->GetCurrentReplay();
        if (engine->GetMaxClients() > 1 && replay->GetViewSize() > 1) {
            if (sar_replay_viewmode.GetInt() == 1) {
                replayRecorder1->StartRecording();
                replayPlayer2->StartPlaying(replay);
            } else {
                replayRecorder2->StartRecording();
                replayPlayer1->StartPlaying(replay);
            }
        }
    }

    if (sar_speedrun_autostart.isRegistered && sar_speedrun_autostart.GetBool() && !speedrun->IsActive()) {
        speedrun->Start(engine->tickcount);
    }

    stepCounter->ResetTimer();

    speedrun->ReloadRules();
    this->currentFrame = 0;
    this->isRunning = true;
}
void Session::Ended()
{
    if (!this->isRunning) {
        return;
    }

    auto tick = engine->GetSessionTick();

    if (tick != 0) {
        console->Print("Session: %i (%.3f)\n", tick, engine->ToTime(tick));
        this->lastSession = tick;
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

    cmdQueuer->Stop();
    replayRecorder1->StopRecording();
    replayRecorder2->StopRecording();
    replayPlayer1->StopPlaying();
    replayPlayer2->StopPlaying();
    speedrun->Pause();
    speedrun->UnloadRules();

    if (listener) {
        listener->Reset();
    }

    this->isRunning = false;
}
void Session::Changed()
{
    console->DevMsg("m_currentState = %i\n", engine->hoststate->m_currentState);

    if (engine->hoststate->m_currentState == HS_CHANGE_LEVEL_SP
        || engine->hoststate->m_currentState == HS_CHANGE_LEVEL_MP
        || engine->hoststate->m_currentState == HS_GAME_SHUTDOWN) {
        this->Ended();
    } else if (engine->hoststate->m_currentState == HS_RUN
        && !engine->hoststate->m_activeGame
        && engine->GetMaxClients() <= 1) {
        this->Started(true);
    }
}
void Session::Changed(int state)
{
    console->DevMsg("state = %i\n", state);

    // Demo recorder starts syncing from this tick
    if (state == SIGNONSTATE_FULL) {
        if (engine->GetMaxClients() <= 1) {
            this->Started();
        }
    } else {
        this->Ended();
    }
}
