#pragma once

#include "TasScript.hpp"
#include "Command.hpp"
#include "Features/Feature.hpp"
#include "Features/Tas/TasController.hpp"
#include "Features/Tas/TasTool.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Client.hpp"
#include "Modules/Server.hpp"

#define TAS_SCRIPTS_DIR "tas"
#define TAS_SCRIPT_EXT "p2tas"

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
	bool ducked;
	bool grounded;
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

	int wasEnginePaused = false; // Used to check if we need to revert incrementing a tick

public:
	void Update();
	void UpdateServer();

	inline int GetTick() const { return currentTick; };
	inline int GetAbsoluteTick() const { return startTick + currentTick; };
	inline int GetStartTick() const { return startTick; };
	inline bool IsActive() const { return active; };
	inline bool IsReady() const { return ready; };
	inline bool IsRunning() const { return active && startTick != -1; }
	inline bool IsPaused() const { return paused; }
	inline bool IsUsingTools() const { 
		return (playbackInfo.slots[0].IsActive() && !playbackInfo.slots[0].IsRaw()) 
			|| (playbackInfo.slots[1].IsActive() && !playbackInfo.slots[1].IsRaw());
	}
	inline int GetScriptVersion(int slot) const { return playbackInfo.slots[slot].header.version; }

	void PlayFile(std::string slot0scriptPath, std::string slot1scriptPath);
	void PlayScript(std::string slot0name, std::string slot0script, std::string slot1name, std::string slot1script);
	void PlaySingleCoop(std::string file, int slot);

	void Activate(TasPlaybackInfo info);
	void Start();
	void PostStart();
	void Stop(bool interrupted=false);
	void Replay(bool automatic=false);

	void Pause();
	void Resume();
	void AdvanceFrame();

	TasFramebulk GetRawFramebulkAt(int slot, int tick);

	TasPlayerInfo GetPlayerInfo(int slot, void *player, CUserCmd *cmd, bool clientside = false);

	void SaveProcessedFramebulks();
	void SaveUsercmdDebugs(int slot);
	void SavePlayerInfoDebugs(int slot);

	void FetchInputs(int slot, TasController *controller);
	void PostProcess(int slot, void *player, CUserCmd *cmd);
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
