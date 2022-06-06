#pragma once

#include "Command.hpp"
#include "Features/Feature.hpp"
#include "Features/Tas/TasController.hpp"
#include "Features/Tas/TasTool.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"

#define TAS_SCRIPTS_DIR "tas"
#define TAS_SCRIPT_EXT "p2tas"

class TasToolCommand;

extern Variable sar_tas_tools_enabled;
extern Variable sar_tas_tools_force;

struct TasFramebulk {
	int tick = 0;
	Vector moveAnalog = {0, 0};
	Vector viewAnalog = {0, 0};
	bool buttonStates[TAS_CONTROLLER_INPUT_COUNT] = {0};
	std::vector<std::string> commands;
	std::vector<TasToolCommand> toolCmds;

	std::string ToString();
};

enum TasStartType {
	ChangeLevel,
	ChangeLevelCM,
	LoadQuicksave,
	StartImmediately,
};

struct TasStartInfo {
	TasStartType type;
	std::string param;
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

class TasPlayer : public Feature {
private:
	bool active = false;
	bool ready = false;
	bool paused = false;
	int startTick = 0;    // used to store the absolute tick in which player started playing the script
	int currentTick = 0;  // tick position of script player, relative to its starting point.
	int lastTick = 0;     // last tick of script, relative to its starting point

	int wasEnginePaused; // Used to check if we need to revert incrementing a tick

	TasStartInfo startInfo;
	std::string tasFileName[2];

	std::vector<TasFramebulk> framebulkQueue[2];
	std::vector<TasFramebulk> processedFramebulks[2];
	std::vector<std::string> usercmdDebugs[2];
	std::vector<std::string> playerInfoDebugs[2];

public:
	void Update();
	void UpdateServer();

	inline int GetTick() const { return currentTick; };
	inline int GetAbsoluteTick() const { return startTick + currentTick; };
	inline int GetStartTick() const { return startTick; };
	inline bool IsActive() const { return active; };
	inline bool IsReady() const { return ready; };
	inline bool IsRunning() const { return active && startTick != -1; }
	inline bool IsUsingTools(int slot) const {
		return sar_tas_tools_enabled.GetBool()
			&& this->tasFileName[slot].size() > 0
			&& (sar_tas_tools_force.GetBool() || this->tasFileName[slot].find("_raw") == std::string::npos);
	}

	void PlayFile(std::string slot0, std::string slot1);
	void PlaySingleCoop(std::string file, int slot);

	void Activate();
	void Start();
	void PostStart();
	void Stop(bool interrupted=false);
	void Replay();

	void Pause();
	void Resume();
	void AdvanceFrame();
	bool IsPaused();

	TasFramebulk GetRawFramebulkAt(int slot, int tick);
	TasPlayerInfo GetPlayerInfo(void *player, CUserCmd *cmd);
	void SetFrameBulkQueue(int slot, std::vector<TasFramebulk> fbQueue);
	void SetStartInfo(TasStartType type, std::string);
	inline void SetLoadedFileName(int slot, std::string name) { tasFileName[slot] = name; };
	void SaveProcessedFramebulks();
	void SaveUsercmdDebugs(int slot);
	void SavePlayerInfoDebugs(int slot);

	void FetchInputs(int slot, TasController *controller);
	void PostProcess(int slot, void *player, CUserCmd *cmd);
	void DumpUsercmd(int slot, const CUserCmd *cmd, int tick, const char *source);
	void DumpPlayerInfo(int slot, int tick, Vector pos, Vector eye_pos, QAngle ang);

	bool isCoop;
	int coopControlSlot;
	bool inControllerCommands = false;
	int numSessionsBeforeStart = 0;
	bool wasStartNext;
	int scriptVersion = 0;
	std::string rngManipFile;

	TasPlayer();
	~TasPlayer();
};

extern Variable sar_tas_debug;
extern Variable sar_tas_autosave_raw;

extern Variable sar_tas_skipto;
extern Variable sar_tas_pauseat;
extern Variable sar_tas_playback_rate;

extern TasPlayer *tasPlayer;
