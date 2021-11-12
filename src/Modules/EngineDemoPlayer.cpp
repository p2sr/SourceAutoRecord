#include "EngineDemoPlayer.hpp"

#include "Checksum.hpp"
#include "Client.hpp"
#include "Console.hpp"
#include "Engine.hpp"
#include "Event.hpp"
#include "Features/Camera.hpp"
#include "Features/Demo/Demo.hpp"
#include "Features/Demo/DemoParser.hpp"
#include "Features/Renderer.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Server.hpp"
#include "Utils.hpp"

#include <filesystem>
#include <vector>

REDECL(EngineDemoPlayer::StartPlayback);
REDECL(EngineDemoPlayer::StopPlayback);
REDECL(EngineDemoPlayer::stopdemo_callback);

static std::vector<std::string> g_demoBlacklist;

Variable sar_demo_blacklist("sar_demo_blacklist", "0", "Stop a set of commands from being run by demo playback.\n");
CON_COMMAND(sar_demo_blacklist_addcmd, "sar_demo_blacklist_addcmd <command> - add a command to the demo blacklist\n") {
	if (args.ArgC() == 1) {
		console->Print(sar_demo_blacklist_addcmd.ThisPtr()->m_pszHelpString);
	} else {
		g_demoBlacklist.push_back({args.m_pArgSBuffer + args.m_nArgv0Size});
	}
}

static bool startsWith(const char *pre, const char *str) {
	while (*pre && *str) {
		if (*pre != *str) {
			return false;
		}
		++pre, ++str;
	}

	return !*pre;
}

bool EngineDemoPlayer::ShouldBlacklistCommand(const char *cmd) {
	if (startsWith("sar_demo_blacklist", cmd)) {
		return true;
	}

	if (sar_demo_blacklist.GetBool()) {
		for (auto &s : g_demoBlacklist) {
			if (startsWith(s.c_str(), cmd)) {
				return true;
			}
		}
	}

	return false;
}

static Variable sar_demo_remove_broken("sar_demo_remove_broken", "1", "Whether to remove broken frames from demo playback\n");

static bool g_waitingForSession = false;
ON_EVENT(SESSION_START) { g_waitingForSession = false; }

static bool g_demoFixing = false;
static int g_demoStart;
void EngineDemoPlayer::HandlePlaybackFix() {
	if (!g_demoFixing) return;

	// 0 = not done anything
	// 1 = queued skip
	// 2 = in skip
	static int state = 0;
	static int nfails = 0;

	int tick = GetTick();

	if (state == 0) {
		if (tick > g_demoStart + 2) {
			console->Print("Beginning playback; skipping\n");
			g_waitingForSession = true;
			engine->ExecuteCommand("demo_resume; demo_gototick 1");
			state = 1;
		}
	} else if (state == 1) {
		if (engine->hoststate->m_currentState == HS_RUN && !g_waitingForSession) {
			state = 2;
		}
	} else if (state == 2) {
		if (!IsSkipping()) {
			if (tick % 2 == g_demoStart % 2) {
				console->Print("Correcting start tick\n");
				engine->SendToCommandBuffer("sv_alternateticks 0", 0);
				engine->SendToCommandBuffer("sv_alternateticks 1", 1);
			}
			console->Print("Successful start\n");
			state = 0;
			g_demoFixing = false;
		}
	}
}
bool EngineDemoPlayer::IsPlaybackFixReady() {
	return !g_demoFixing;
}

int EngineDemoPlayer::GetTick() {
	return this->GetPlaybackTick(this->s_ClientDemoPlayer->ThisPtr());
}
bool EngineDemoPlayer::IsPlaying() {
	return this->IsPlayingBack(this->s_ClientDemoPlayer->ThisPtr());
}
bool EngineDemoPlayer::IsPaused() {
	return this->IsPlaybackPaused(this->s_ClientDemoPlayer->ThisPtr());
}
bool EngineDemoPlayer::IsSkipping() {
	return this->IsSkipping_(this->s_ClientDemoPlayer->ThisPtr());
}

void EngineDemoPlayer::SkipTo(int tick, bool relative, bool pause) {
	this->SkipToTick(this->s_ClientDemoPlayer->ThisPtr(), tick, relative, pause);
}

void EngineDemoPlayer::ClearDemoQueue() {
	engine->demoplayer->demoQueue.clear();
	engine->demoplayer->demoQueueSize = 0;
	engine->demoplayer->currentDemoID = -1;
}

std::string EngineDemoPlayer::GetLevelName() {
	return this->levelName;
}

// 0x01: timescale detection
// 0x02: initial cvar
// 0x03: entity input
// 0x04: entity input triggered by slot
// 0x05: portal placement
// 0x06: hit cm flags
// 0x07: got crouchfly
// 0x08: pause duration
// 0x09: 'wait' run
// 0x0A: speedrun finish
void EngineDemoPlayer::CustomDemoData(char *data, size_t length) {
	if (data[0] == 0x03 || data[0] == 0x04) {  // Entity input data
		std::optional<int> slot;
		if (data[0] == 0x04) {
			slot = data[1];
			++data;
		}
		char *targetname = data + 1;
		size_t targetnameLen = strlen(targetname);
		char *classname = data + 2 + targetnameLen;
		size_t classnameLen = strlen(classname);
		char *inputname = data + 3 + targetnameLen + classnameLen;
		size_t inputnameLen = strlen(inputname);
		char *parameter = data + 4 + targetnameLen + classnameLen + inputnameLen;

		//console->Print("[%4d] %s.%s(%s)\n", this->GetTick(), targetname, inputname, parameter);

		SpeedrunTimer::TestInputRules(targetname, classname, inputname, parameter, slot);

		return;
	}

	if (data[0] == 0x05) {  // Portal placement
		int slot = data[1];
		PortalColor portal = data[2] ? PortalColor::ORANGE : PortalColor::BLUE;
		Vector pos;
		pos.x = *(float *)(data + 3);
		pos.y = *(float *)(data + 7);
		pos.z = *(float *)(data + 11);

		SpeedrunTimer::TestPortalRules(pos, slot, portal);

		return;
	}

	if (data[0] == 0x06) {  // CM flags
		int slot = data[1];

		SpeedrunTimer::TestFlagRules(slot);

		return;
	}

	if (data[0] == 0x07) {  // Crouch fly
		int slot = data[1];

		SpeedrunTimer::TestFlyRules(slot);

		return;
	}
}

DETOUR_COMMAND(EngineDemoPlayer::stopdemo) {
	engine->demoplayer->ClearDemoQueue();
	EngineDemoPlayer::stopdemo_callback(args);
}

// CDemoPlayer::StartPlayback
DETOUR(EngineDemoPlayer::StartPlayback, const char *filename, bool bAsTimeDemo) {
	auto result = EngineDemoPlayer::StartPlayback(thisptr, filename, bAsTimeDemo);

	if (result) {
		DemoParser parser;
		Demo demo;
		auto dir = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(engine->demoplayer->DemoName);
		if (parser.Parse(dir, &demo)) {
			parser.Adjust(&demo);
			console->Print("Client:   %s\n", demo.clientName);
			console->Print("Map:      %s\n", demo.mapName);
			console->Print("Ticks:    %i\n", demo.playbackTicks);
			console->Print("Time:     %.3f\n", demo.playbackTime);
			console->Print("Tickrate: %.3f\n", demo.Tickrate());
			engine->demoplayer->levelName = demo.mapName;
			g_demoStart = demo.firstPositivePacketTick;
			Renderer::segmentEndTick = demo.segmentTicks;
			Event::Trigger<Event::DEMO_START>({});
			g_demoFixing = sar_demo_remove_broken.GetBool();
		} else {
			console->Print("Could not parse \"%s\"!\n", engine->demoplayer->DemoName);
		}
	}

	camera->RequestTimeOffsetRefresh();

	Renderer::isDemoLoading = true;

	return result;
}

// CDemoPlayer::StopPlayback
DETOUR(EngineDemoPlayer::StopPlayback) {
	if (engine->demoplayer->IsPlaying()) {
		Event::Trigger<Event::DEMO_STOP>({});
	}
	return EngineDemoPlayer::StopPlayback(thisptr);
}

bool EngineDemoPlayer::Init() {
	auto disconnect = engine->cl->Original(Offsets::Disconnect);
	void *demoplayer;
#ifndef _WIN32
	if (sar.game->Is(SourceGame_EIPRelPIC)) {
		demoplayer = *(void **)(disconnect + 10 + *(uint32_t *)(disconnect + 12) + *(uint32_t *)(disconnect + 100));
	} else
#endif
		demoplayer = Memory::DerefDeref<void *>(disconnect + Offsets::demoplayer);
	if (this->s_ClientDemoPlayer = Interface::Create(demoplayer)) {
		this->s_ClientDemoPlayer->Hook(EngineDemoPlayer::StartPlayback_Hook, EngineDemoPlayer::StartPlayback, Offsets::StartPlayback);
		this->s_ClientDemoPlayer->Hook(EngineDemoPlayer::StopPlayback_Hook, EngineDemoPlayer::StopPlayback, Offsets::StopPlayback);

		this->GetPlaybackTick = s_ClientDemoPlayer->Original<_GetPlaybackTick>(Offsets::GetPlaybackTick);
		this->IsPlayingBack = s_ClientDemoPlayer->Original<_IsPlayingBack>(Offsets::IsPlayingBack);
		this->IsPlaybackPaused = s_ClientDemoPlayer->Original<_IsPlaybackPaused>(Offsets::IsPlaybackPaused);
		this->IsSkipping_ = s_ClientDemoPlayer->Original<_IsSkipping>(Offsets::IsSkipping);
		this->SkipToTick = s_ClientDemoPlayer->Original<_SkipToTick>(Offsets::SkipToTick);
		this->DemoName = reinterpret_cast<char *>((uintptr_t)demoplayer + Offsets::m_szFileName);
	}

	Command::Hook("stopdemo", EngineDemoPlayer::stopdemo_callback_hook, EngineDemoPlayer::stopdemo_callback);

	this->currentDemoID = -1;
	this->demoQueueSize = 0;

	return this->hasLoaded = this->s_ClientDemoPlayer;
}
void EngineDemoPlayer::Shutdown() {
	Interface::Delete(this->s_ClientDemoPlayer);
	Command::Unhook("stopdemo", EngineDemoPlayer::stopdemo_callback);
}

// Commands

CON_COMMAND_AUTOCOMPLETEFILE(sar_startdemos, "sar_startdemos <demoname> - improved version of startdemos. Use 'stopdemo' to stop playing demos\n", 0, 0, dem) {
	// Always print a useful message for the user if not used correctly
	if (args.ArgC() <= 1) {
		return console->Print(sar_startdemos.ThisPtr()->m_pszHelpString);
	}

	engine->demoplayer->demoQueue.clear();

	std::string name = args[1];

	if (name.length() > 4) {
		if (name.substr(name.length() - 4, 4) == ".dem")
			name.resize(name.length() - 4);
	}

	auto dir = engine->GetGameDirectory() + std::string("/");
	int counter = 2;

	Demo demo;
	DemoParser parser;
	bool ok = parser.Parse(dir + name, &demo);

	if (!ok) {
		return console->Print("Could not parse \"%s\"!\n", engine->GetGameDirectory() + std::string("/") + args[1]);
	}

	engine->demoplayer->demoQueue.push_back(name);

	while (ok) {
		auto tmp_dir = dir + name + "_" + std::to_string(counter);
		ok = parser.Parse(tmp_dir, &demo);
		if (ok) {
			engine->demoplayer->demoQueue.push_back(name + "_" + std::to_string(counter));
		}
		++counter;
	}

	engine->demoplayer->demoQueueSize = engine->demoplayer->demoQueue.size();
	engine->demoplayer->currentDemoID = 0;

	EngineDemoPlayer::stopdemo_callback(args);

	//Demos are played in Engine::Frame
}
CON_COMMAND(sar_startdemosfolder, "sar_startdemosfolder <folder name> - plays all the demos in the specified folder by alphabetic order\n") {
	if (args.ArgC() < 2) {
		return console->Print(sar_startdemosfolder.ThisPtr()->m_pszHelpString);
	}

	engine->demoplayer->demoQueue.clear();

	auto dir = engine->GetGameDirectory() + std::string("/");
	std::string filepath;
	Demo demo;
	DemoParser parser;

	for (const auto &file : std::filesystem::directory_iterator(dir + args[1])) {
		if (file.path().extension() != ".dem")
			continue;

		filepath = args[1] + std::string("/") + file.path().filename().string();
		console->Print("%s\n", filepath.c_str());
		if (parser.Parse(dir + filepath, &demo)) {
			engine->demoplayer->demoQueue.push_back(filepath);
		}
	}

	std::sort(engine->demoplayer->demoQueue.begin(), engine->demoplayer->demoQueue.end());

	engine->demoplayer->demoQueueSize = engine->demoplayer->demoQueue.size();
	engine->demoplayer->currentDemoID = 0;

	EngineDemoPlayer::stopdemo_callback(args);
}
CON_COMMAND_COMPLETION(sar_skiptodemo, "sar_skiptodemo <demoname> - skip demos in demo queue to this demo\n", ({engine->demoplayer->demoQueue})) {
	if (args.ArgC() < 2) {
		return console->Print(sar_skiptodemo.ThisPtr()->m_pszHelpString);
	}

	auto it = std::find(engine->demoplayer->demoQueue.begin(), engine->demoplayer->demoQueue.end(), args[1]);
	if (it == engine->demoplayer->demoQueue.end())
		return;

	engine->demoplayer->currentDemoID = std::distance(engine->demoplayer->demoQueue.begin(), it);

	EngineDemoPlayer::stopdemo_callback(args);
}
CON_COMMAND(sar_nextdemo, "sar_nextdemo - plays the next demo in demo queue\n") {
	if (++engine->demoplayer->currentDemoID >= engine->demoplayer->demoQueueSize) {
		return engine->demoplayer->ClearDemoQueue();
	}

	EngineDemoPlayer::stopdemo_callback(args);
}
