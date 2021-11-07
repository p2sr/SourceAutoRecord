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
	WaitForNewSession,
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

	int pauseTick = 0;  // the tick TasPlayer should pause the game at, used for frame advancing.

	int wasEnginePaused; // Used to check if we need to revert incrementing a tick

	TasStartInfo startInfo;
	std::string tasFileName;

	std::vector<TasFramebulk> framebulkQueue;
	std::vector<TasFramebulk> processedFramebulks;

public:
	void Update();

	inline int GetTick() const { return currentTick; };
	inline int GetAbsoluteTick() const { return startTick + currentTick; };
	inline bool IsActive() const { return active; };
	inline bool IsRunning() const { return active && startTick != -1; }

	void Activate();
	void Start();
	void PostStart();
	void Stop(bool interrupted=false);

	void Pause();
	void Resume();
	void AdvanceFrame();
	bool IsPaused();

	TasFramebulk GetRawFramebulkAt(int tick);
	TasPlayerInfo GetPlayerInfo(void *player, CMoveData *pMove);
	void SetFrameBulkQueue(std::vector<TasFramebulk> fbQueue);
	void SetStartInfo(TasStartType type, std::string);
	inline void SetLoadedFileName(std::string name) { tasFileName = name; };
	void SaveProcessedFramebulks();


	void FetchInputs(TasController *controller);
	void PostProcess(void *player, CMoveData *pMove);

	TasPlayer();
	~TasPlayer();
};

extern Variable sar_tas_debug;
extern Variable sar_tas_tools_enabled;
extern Variable sar_tas_autosave_raw;

extern Variable sar_tas_skipto;
extern Variable sar_tas_pauseat;

extern TasPlayer *tasPlayer;
