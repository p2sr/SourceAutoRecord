#pragma once
#include "Interface.hpp"
#include "Module.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

#include <map>

class EngineDemoPlayer : public Module {
public:
	Interface *s_ClientDemoPlayer = nullptr;

	using _IsPlayingBack = bool(__rescall *)(void *thisptr);
	using _IsPlaybackPaused = bool(__rescall *)(void *thisptr);
	using _IsSkipping = bool(__rescall *)(void *thisptr);
	using _GetPlaybackTick = int(__rescall *)(void *thisptr);
	using _SkipToTick = int(__rescall *)(void *thisptr, int tick, bool relative, bool pause);

	_IsPlayingBack IsPlayingBack = nullptr;
	_IsPlaybackPaused IsPlaybackPaused = nullptr;
	_IsSkipping IsSkipping_ = nullptr;
	_GetPlaybackTick GetPlaybackTick = nullptr;
	_SkipToTick SkipToTick = nullptr;

	char *DemoName = nullptr;
	int demoQueueSize = false;
	int currentDemoID = false;
	std::vector<std::string> demoQueue;
	std::string levelName;

public:
	int GetTick();
	bool IsPlaying();
	bool IsPaused();
	bool IsSkipping();
	void ClearDemoQueue();
	std::string GetLevelName();
	void CustomDemoData(char *data, size_t length);
	void SkipTo(int tick, bool relative, bool pause);

	void HandlePlaybackFix();
	bool IsPlaybackFixReady();

	// CDemoRecorder::StartPlayback
	DECL_DETOUR(StartPlayback, const char *filename, bool bAsTimeDemo);
	// CDemoRecorder::StopPlayback
	DECL_DETOUR(StopPlayback);
	DECL_DETOUR_COMMAND(stopdemo);

	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE("engine"); }

	bool ShouldBlacklistCommand(const char *cmd);
};

extern Command sar_startdemos;
extern Command sar_startdemosfolder;
extern Command sar_skiptodemo;
extern Command sar_nextdemo;
