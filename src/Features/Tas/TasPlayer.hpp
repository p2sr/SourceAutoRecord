#pragma once

#include "Command.hpp"
#include "Features/Feature.hpp"
#include "Features/Tas/TasController.hpp"
#include "Features/Tas/TasTool.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "TasScript.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"

#define TAS_SCRIPTS_DIR "tas"
#define TAS_SCRIPT_EXT "p2tas"

#define TAS_SCRIPT_VERSION_AT_LEAST(version) (tasPlayer->GetScriptVersion(slot) >= version)
#define TAS_SCRIPT_VERSION_AT_MOST(version) (tasPlayer->GetScriptVersion(slot) <= version)
#define FOR_TAS_SCRIPT_VERSIONS_SINCE(version) if (TAS_SCRIPT_VERSION_AT_LEAST(version))
#define FOR_TAS_SCRIPT_VERSIONS_UNTIL(version) if (TAS_SCRIPT_VERSION_AT_MOST(version))


class TasToolCommand;

extern Variable sar_tas_tools_enabled;
extern Variable sar_tas_tools_force;

struct TasPlaybackInfo {
	TasScript slots[2];
	int replayCount = 0;
	int autoReplayCount = 0;

	int coopControlSlot = -1;

	inline bool IsCoop() const { return slots[1].IsActive() || coopControlSlot == 1; }
	inline bool HasActiveSlot() const { return slots[0].IsActive() || slots[1].IsActive(); }
	inline TasScript GetMainScript() const {
		if (coopControlSlot >= 0 && slots[1 - coopControlSlot].IsActive()) {
			return slots[1 - coopControlSlot];
		}
		return slots[0].IsActive() ? slots[0] : slots[1];
	}
	inline TasScriptHeader GetMainHeader() const { return GetMainScript().header; }
};

struct TasPlayerInfo {
	int slot;
	int tick;
	Vector position;
	QAngle angles;
	Vector velocity;
	float surfaceFriction;
	float maxSpeed;
	int waterLevel;
	bool ducked;
	bool grounded;
	bool willBeGrounded;
	bool onSpeedPaint;
	int oldButtons;
	float ticktime;
};

class TasPlayer;
extern TasPlayer *tasPlayer;

class TasPlayer : public Feature {
private:
	bool active = false;
	bool ready = false;
	bool paused = false;
	int startTick = 0;    // used to store the absolute tick in which player started playing the script
	int currentTick = 0;  // tick position of script player, relative to its starting point.
	int lastTick = 0;     // last tick of script, relative to its starting point

	int wasEnginePaused = false;  // Used to check if we need to revert incrementing a tick

	// used to cache last used framebulk to quickly access it for playback
	unsigned currentRequestRawFramebulkIndex[2];

public:
	void Update();
	void UpdateServer();

	inline int GetTick() const { return currentTick; };
	inline int GetAbsoluteTick() const { return startTick + currentTick; };
	inline int GetStartTick() const { return startTick; };
	inline std::string GetScriptName(int slot) const { return playbackInfo.slots[slot].name; };
	inline bool IsActive() const { return active; };
	inline bool IsReady() const { return ready; };
	inline bool IsRunning() const { return active && startTick != -1; }
	inline bool IsPaused() const { return paused; }
	inline bool IsUsingTools() const {
		return (playbackInfo.slots[0].IsActive() && !playbackInfo.slots[0].IsRaw()) || (playbackInfo.slots[1].IsActive() && !playbackInfo.slots[1].IsRaw());
	}
	inline int GetScriptVersion(int slot) const { return playbackInfo.slots[slot].header.version; }

	void PlayFile(std::string slot0scriptPath, std::string slot1scriptPath);
	void PlayScript(std::string slot0name, std::string slot0script, std::string slot1name, std::string slot1script);
	void PlaySingleCoop(std::string file, int slot);

	void Activate(TasPlaybackInfo info);
	void Start();
	void PostStart();
	void Stop(bool interrupted = false);
	void Replay(bool automatic = false);

	void Pause();
	void Resume();
	void AdvanceFrame();

	TasFramebulk GetRawFramebulkAt(int slot, int tick);
	TasFramebulk GetRawFramebulkAt(int slot, int tick, unsigned &cachedIndex);
	TasFramebulk &RequestProcessedFramebulkAt(int slot, int tick);

	TasPlayerInfo GetPlayerInfo(int slot, void *player, CUserCmd *cmd, bool clientside = false);
	int FetchCurrentPlayerTickBase(void *player, bool clientside = false);

	void SaveProcessedFramebulks();
	void SaveUsercmdDebugs(int slot);
	void SavePlayerInfoDebugs(int slot);

	void FetchInputs(int slot, TasController *controller, CUserCmd *cmd);
	TasFramebulk SamplePreProcessedFramebulk(int slot, int tasTick, void *player, CUserCmd *cmd);
	void PostProcess(int slot, void *player, CUserCmd *cmd);
	void ApplyMoveAnalog(Vector moveAnalog, CUserCmd *cmd);
	void UpdateTools(int slot, const TasFramebulk &fb, TasToolProcessingType processType);
	void ApplyTools(TasFramebulk &fb, const TasPlayerInfo &pInfo, TasToolProcessingType processType);
	bool CanProcessTool(TasTool *tool, TasToolProcessingType processType);
	void DumpUsercmd(int slot, const CUserCmd *cmd, int tick, const char *source);
	void DumpPlayerInfo(int slot, int tick, Vector pos, Vector eye_pos, QAngle ang);

	bool inControllerCommands = false;
	int numSessionsBeforeStart = 0;

	TasPlaybackInfo previousPlaybackInfo;
	TasPlaybackInfo playbackInfo;

	TasPlayer();
	~TasPlayer();
};

extern Variable sar_tas_debug;
extern Variable sar_tas_autosave_raw;

extern Variable sar_tas_skipto;
extern Variable sar_tas_pauseat;
extern Variable sar_tas_playback_rate;
