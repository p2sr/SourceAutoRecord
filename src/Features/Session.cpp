#include "Session.hpp"

#include "Event.hpp"
#include "Features/Demo/DemoGhostPlayer.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/Listener.hpp"
#include "Features/NetMessage.hpp"
#include "Features/ReplaySystem/ReplayPlayer.hpp"
#include "Features/ReplaySystem/ReplayProvider.hpp"
#include "Features/ReplaySystem/ReplayRecorder.hpp"
#include "Features/SegmentedTools.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Stats/Stats.hpp"
#include "Features/StepCounter.hpp"
#include "Features/Summary.hpp"
#include "Features/Timer/Timer.hpp"
#include "Features/TimescaleDetect.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Utils/SDK.hpp"

#include <chrono>
#include <thread>

Variable sar_loads_uncap("sar_loads_uncap", "0", 0, 1, "Temporarily set fps_max to 0 during loads\n");
Variable sar_loads_norender("sar_loads_norender", "0", 0, 1, "Temporarily set mat_noredering to 1 during loads\n");

Variable sar_load_delay("sar_load_delay", "0", 0, "Delay for this number of milliseconds at the end of a load.\n");

Session *session;

Session::Session()
	: baseTick(0)
	, lastSession(0)
	, isRunning(true)
	, currentFrame(0)
	, lastFrame(0)
	, prevState(HS_RUN)
	, signonState(SIGNONSTATE_FULL) {
	this->hasLoaded = true;
}
int Session::GetTick() {
	auto result = engine->GetTick() - this->baseTick;
	return (result >= 0) ? result : 0;
}
void Session::Rebase(const int from) {
	this->baseTick = from;
}
void Session::Started(bool menu) {
	if (this->isRunning) {
		return;
	}

	if (menu) {
		console->Print("Session started! (menu)\n");
		this->Rebase(engine->GetTick());

		if (sar_speedrun_stop_in_menu.isRegistered && sar_speedrun_stop_in_menu.GetBool()) {
			SpeedrunTimer::Stop("menu");
		} else {
			SpeedrunTimer::Resume();
		}

		if (!engine->IsOrange()) {
			this->ResetLoads();
		}

		this->isRunning = true;
	} else {
		console->Print("Session Started!\n");
		this->Start();
	}
}
void Session::Start() {
	if (this->isRunning) {
		return;
	}

	auto tick = engine->GetTick();

	this->Rebase(tick);
	timer->Rebase(tick);

	Event::Trigger<Event::SESSION_START>({});

	engine->hasRecorded = false;
	engine->hasPaused = false;
	engine->isPausing = false;
	engine->startedTransitionFadeout = false;
	engine->forcedPrimaryFullscreen = false;
	server->tickCount = 0;

	this->currentFrame = 0;
	this->isRunning = true;
}
void Session::Ended() {
	if (!this->isRunning) {
		return;
	}

	this->previousMap = engine->GetCurrentMapName();

	auto tick = this->GetTick();

	Event::Trigger<Event::SESSION_END>({});

	if (tick != 0) {
		console->Print("Session: %i (%.3f)\n", tick, engine->ToTime(tick));
		this->lastSession = tick;
	}

	if (summary->isRunning) {
		summary->Add(tick, engine->ToTime(tick), engine->GetCurrentMapName().c_str());
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

	replayRecorder1->StopRecording();
	replayRecorder2->StopRecording();
	replayPlayer1->StopPlaying();
	replayPlayer2->StopPlaying();

	NetMessage::SessionEnded();

	// This pause generally won't do anything in co-op; it will have
	// already happened in the playvideo_end_level_transition detour.
	// However, if a level ends prematurely (e.g. restart_level), that
	// command is never run, so we use session timing to pause instead
	if (!engine->IsOrange()) {
		SpeedrunTimer::Pause();
	}

	if (listener) {
		listener->Reset();
	}

	auto nSlot = GET_SLOT();
	stats->Get(nSlot)->statsCounter->RecordDatas(tick);

	demoGhostPlayer.DeleteAllGhostModels();
	networkManager.DeleteAllGhosts();

	if (networkManager.isConnected) networkManager.splitTicks = -1;

	this->loadStart = NOW();
	if (!engine->demoplayer->IsPlaying() && !engine->IsOrange()) {
		this->DoFastLoads();
	}

	this->isRunning = false;
}
void Session::Changed() {
	console->DevMsg("m_currentState = %i\n", engine->hoststate->m_currentState);

	if (engine->hoststate->m_currentState == HS_CHANGE_LEVEL_SP || engine->hoststate->m_currentState == HS_CHANGE_LEVEL_MP || engine->hoststate->m_currentState == HS_GAME_SHUTDOWN) {
		this->Ended();
	} else if (engine->hoststate->m_currentState == HS_RUN && !engine->hoststate->m_activeGame && engine->GetMaxClients() <= 1) {
		this->Started(true);
	}
}
void Session::Changed(int state) {
	console->DevMsg("state = %i\n", state);
	this->signonState = state;

	// Ghosts are in saves, and just sorta stay there unless we kill
	// them. We have to do this in prespawn - if we do it at session
	// start, it kills ghosts that were just spawned and hence
	// invalidates entity pointers and bad things happen
	if (state == SIGNONSTATE_PRESPAWN && networkManager.isConnected) {
		GhostEntity::KillAllGhosts();
	}

	// Demo recorder starts syncing from this tick
	if (state == SIGNONSTATE_FULL) {
		timescaleDetect->Spawn();
		client->FlushChatQueue();
		this->Started();
		this->loadEnd = NOW();

		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(this->loadEnd - this->loadStart).count();
		console->DevMsg("Load took: %dms\n", time);

		if (sar_load_delay.GetInt()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(sar_load_delay.GetInt()));
		}
	} else if (state == SIGNONSTATE_PRESPAWN) {
		this->ResetLoads();
		SpeedrunTimer::FinishLoad();
	} else {
		this->Ended();
	}
}

void Session::DoFastLoads() {
	if (sar_loads_uncap.GetBool()) {
		this->oldFpsMax = fps_max.GetInt();
		fps_max.SetValue(0);
	}

	if (sar_loads_norender.GetBool()) {
		mat_norendering.SetValue(1);
	}
}

void Session::ResetLoads() {
	if (sar_loads_uncap.GetBool() && fps_max.GetInt() == 0) {
		fps_max.SetValue(this->oldFpsMax);
	}

	if (sar_loads_norender.GetBool()) {
		mat_norendering.SetValue(0);
	}
}

// HUD

HUD_ELEMENT2(session, "0", "Draws current session tick.\n", HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen) {
	auto tick = (session->isRunning) ? session->GetTick() : 0;
	ctx->DrawElement("session: %i (%.3f)", tick, engine->ToTime(tick));
}
HUD_ELEMENT2(last_session, "0", "Draws value of latest completed session.\n", HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen) {
	ctx->DrawElement("last session: %i (%.3f)", session->lastSession, engine->ToTime(session->lastSession));
}
HUD_ELEMENT2(sum, "0", "Draws summary value of sessions.\n", HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen) {
	if (summary->isRunning && sar_sum_during_session.GetBool()) {
		auto tick = (session->isRunning) ? session->GetTick() : 0;
		auto time = engine->ToTime(tick);
		ctx->DrawElement("sum: %i (%.3f)", summary->totalTicks + tick, engine->ToTime(summary->totalTicks) + time);
	} else {
		ctx->DrawElement("sum: %i (%.3f)", summary->totalTicks, engine->ToTime(summary->totalTicks));
	}
}
HUD_ELEMENT2(frame, "0", "Draws current frame count.\n", HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen) {
	ctx->DrawElement("frame: %i", session->currentFrame);
}
HUD_ELEMENT2(last_frame, "0", "Draws last saved frame value.\n", HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen) {
	ctx->DrawElement("last frame: %i", session->lastFrame);
}
