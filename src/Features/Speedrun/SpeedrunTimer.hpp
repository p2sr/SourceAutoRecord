#pragma once

#include "Categories.hpp"
#include "Command.hpp"
#include "Rules.hpp"
#include "Variable.hpp"

#include <string>

#define SPEEDRUN_TOAST_TAG "speedrun"

namespace SpeedrunTimer {
	std::string Format(float raw);
	std::string SimpleFormat(float raw);
	float UnFormat(std::string &formated_time);

	void Init();
	void SetIpt(float ipt);
	void Update();
	void AddPauseTick();
	void FinishLoad();

	int GetSegmentTicks();
	int GetSplitTicks();
	int GetTotalTicks();

	void Start();
	void Pause();
	void Resume();
	void Stop(std::string segName);
	void Split(bool newSplit, std::string segName, bool requested = true);
	void Reset(bool requested = true);

	bool IsRunning();

	void OnLoad();
	void CategoryChanged();
};  // namespace SpeedrunTimer

extern Variable sar_speedrun_smartsplit;
extern Variable sar_speedrun_time_pauses;
extern Variable sar_speedrun_stop_in_menu;
extern Variable sar_speedrun_start_on_load;
extern Variable sar_speedrun_offset;
extern Variable sar_speedrun_autostop;

extern Command sar_speedrun_start;
extern Command sar_speedrun_stop;
extern Command sar_speedrun_split;
extern Command sar_speedrun_pause;
extern Command sar_speedrun_resume;
extern Command sar_speedrun_reset;
extern Command sar_speedrun_result;
extern Command sar_speedrun_export;
