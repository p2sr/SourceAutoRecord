#include "Session.hpp"

#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Features/Demo/DemoGhostPlayer.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/Listener.hpp"
#include "Features/Rebinder.hpp"
#include "Features/ReplaySystem/ReplayPlayer.hpp"
#include "Features/ReplaySystem/ReplayProvider.hpp"
#include "Features/ReplaySystem/ReplayRecorder.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Stats/Stats.hpp"
#include "Features/Stats/ZachStats.hpp"
#include "Features/StepCounter.hpp"
#include "Features/Summary.hpp"
#include "Features/Tas/CommandQueuer.hpp"
#include "Features/Timer/Timer.hpp"
#include "Features/TimescaleDetect.hpp"
#include "Features/SegmentedTools.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Utils/SDK.hpp"

Variable sar_shane_loads("sar_shane_loads", "0", 0, 1, "Temporarily sets fps_max to 0 and mat_noredering to 1 during loads\n");
Variable sar_shane_norendering("sar_shane_norendering", "0", 0, 1, "Temporatily set mat_noredering to 1 during loads\n");

Session* session;

Session::Session()
    : baseTick(0)
    , lastSession(0)
    , isRunning(true)
    , currentFrame(0)
    , lastFrame(0)
    , prevState(HS_RUN)
    , signonState(SIGNONSTATE_FULL)
{
    this->hasLoaded = true;
}
int Session::GetTick()
{
    auto result = engine->GetTick() - this->baseTick;
    return (result >= 0) ? result : 0;
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
        this->Rebase(engine->GetTick());

        if (sar_speedrun_autostop.isRegistered && sar_speedrun_autostop.GetBool()) {
            speedrun->Stop(false);
        } else {
            speedrun->Resume(engine->GetTick());
        }

        if (sar_shane_loads.GetBool()) {
            this->ResetLoads();
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

    auto tick = engine->GetTick();

    this->Rebase(tick);
    timer->Rebase(tick);
    speedrun->Resume(tick);

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
        speedrun->Start(engine->GetTick());
    } else if (speedrun->IsActive()) {
        speedrun->Resume(engine->GetTick());
    }

    //Network Ghosts
    if (networkManager.isConnected) {
        networkManager.NotifyMapChange();
        networkManager.UpdateGhostsSameMap();
        networkManager.SpawnAllGhosts();
        if (ghost_sync.GetBool()) {
            if (!networkManager.AreAllGhostsOnSameMap() && this->previousMap != engine->m_szLevelName) { //Don't pause if just reloading save
                engine->SendToCommandBuffer("pause", 20);
            }
        }
    }

    //Demo ghosts
    if (demoGhostPlayer.IsPlaying()) {
        demoGhostPlayer.UpdateGhostsSameMap();
        if (demoGhostPlayer.IsFullGame()) {
            if (ghost_sync.GetBool()) {
                demoGhostPlayer.Sync();
            }
        } else {
            demoGhostPlayer.ResetAllGhosts();
            demoGhostPlayer.ResumeAllGhosts();
        }
        demoGhostPlayer.SpawnAllGhosts();
    }

    zachStats->ResetTriggers();
    zachStats->NewSession();

    engine->hasRecorded = false;
    engine->hasPaused = false;

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

    this->previousMap = engine->m_szLevelName;

    auto tick = this->GetTick();

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
            timer->Save(engine->GetTick());
            console->Print("Timer paused: %i (%.3f)!\n", timer->totalTicks, engine->ToTime(timer->totalTicks));
        } else {
            timer->Stop(engine->GetTick());
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

    auto nSlot = GET_SLOT();
    stats->Get(nSlot)->statsCounter->RecordDatas(tick);

    networkManager.DeleteAllGhosts();
    
    engine->hasWaited = false;

    this->loadStart = NOW();
    if (sar_shane_loads.GetBool() && !engine->demoplayer->IsPlaying()) {
        this->oldFpsMax = fps_max.GetInt();
        this->DoFastLoads();
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
    this->signonState = state;
    // Demo recorder starts syncing from this tick
    if (state == SIGNONSTATE_FULL) {
        timescaleDetect->Spawn();
        if (engine->GetMaxClients() <= 1) {
            this->Started();
            this->loadEnd = NOW();

            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(this->loadEnd - this->loadStart).count();
            console->DevMsg("Load took : %dms\n", time);
        }
    } else if (state == SIGNONSTATE_PRESPAWN && sar_shane_loads.GetBool()) {
        this->ResetLoads();
    } else {
        this->Ended();
    }
}

void Session::DoFastLoads()
{
    fps_max.SetValue(0);
    if(sar_shane_norendering.GetBool())
        mat_norendering.SetValue(1);
}

void Session::ResetLoads()
{
    fps_max.SetValue(this->oldFpsMax);
    if (sar_shane_norendering.GetBool())
        mat_norendering.SetValue(0);
}

// HUD

HUD_ELEMENT(session, "0", "Draws current session tick.\n", HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen)
{
    auto tick = (session->isRunning) ? session->GetTick() : 0;
    ctx->DrawElement("session: %i (%.3f)", tick, engine->ToTime(tick));
}
HUD_ELEMENT(last_session, "0", "Draws value of latest completed session.\n", HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen)
{
    ctx->DrawElement("last session: %i (%.3f)", session->lastSession, engine->ToTime(session->lastSession));
}
HUD_ELEMENT(sum, "0", "Draws summary value of sessions.\n", HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen)
{
    if (summary->isRunning && sar_sum_during_session.GetBool()) {
        auto tick = (session->isRunning) ? session->GetTick() : 0;
        auto time = engine->ToTime(tick);
        ctx->DrawElement("sum: %i (%.3f)", summary->totalTicks + tick, engine->ToTime(summary->totalTicks) + time);
    } else {
        ctx->DrawElement("sum: %i (%.3f)", summary->totalTicks, engine->ToTime(summary->totalTicks));
    }
}
HUD_ELEMENT(frame, "0", "Draws current frame count.\n", HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen)
{
    ctx->DrawElement("frame: %i", session->currentFrame);
}
HUD_ELEMENT(last_frame, "0", "Draws last saved frame value.\n", HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen)
{
    ctx->DrawElement("last frame: %i", session->lastFrame);
}
