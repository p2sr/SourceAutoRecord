#pragma once

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

	template <bool serverside>
	TasPlayerInfo GetPlayerInfo(void *player, CUserCmd *cmd) {
		using Ent = std::conditional_t<serverside, ServerEnt, ClientEnt>;
		Ent *pl = (Ent *)player;

		TasPlayerInfo pi;

		int m_nOldButtons = pl->template field<int>("m_nOldButtons");

		pi.tick = pl->template field<int>("m_nTickBase");
		pi.slot = server->GetSplitScreenPlayerSlot(player);
		int m_surfaceFriction = serverside ? Offsets::S_m_surfaceFriction : Offsets::C_m_surfaceFriction;
		pi.surfaceFriction = *reinterpret_cast<float *>((uintptr_t)player + m_surfaceFriction);
		pi.ducked = pl->ducked();

		float *m_flMaxspeed = &pl->template field<float>("m_flMaxspeed");
		pi.maxSpeed = *m_flMaxspeed;

		if (serverside) {
		#ifdef _WIN32
			// windows being weird. ask mlugg for explanation because idfk.
			void *paintPowerUser = (void *)((uint32_t)player + 0x1250);
		#else
			void *paintPowerUser = player;
		#endif
			using _GetPaintPower = const PaintPowerInfo_t &(__rescall *)(void *thisptr, unsigned paintId);
			_GetPaintPower GetPaintPower = Memory::VMT<_GetPaintPower>(paintPowerUser, Offsets::GetPaintPower);
			PaintPowerInfo_t speedPaintInfo = GetPaintPower(paintPowerUser, 2);
			pi.onSpeedPaint = speedPaintInfo.m_State == 1;  // ACTIVE_PAINT_POWER

			if (pi.onSpeedPaint) {
				// maxSpeed is modified within ProcessMovement. This hack allows us to "predict" its next value
				// Cache off old max speed to restore later
				float oldMaxSpeed = *m_flMaxspeed;
				// Use the speed paint to modify the max speed
				using _UseSpeedPower = void(__rescall *)(void *thisptr, PaintPowerInfo_t &info);
				_UseSpeedPower UseSpeedPower = Memory::VMT<_UseSpeedPower>(player, Offsets::UseSpeedPower);
				UseSpeedPower(player, speedPaintInfo);
				// Get the new ("predicted") max speed and restore the old one on the player
				pi.maxSpeed = *m_flMaxspeed;
				*m_flMaxspeed = oldMaxSpeed;
			}
		}

		pi.grounded = pl->ground_entity();

		// this check was originally broken, so bypass it in v1
		if (tasPlayer->scriptVersion >= 2) {
			// predict the grounded state after jump.
			if (pi.grounded && (cmd->buttons & IN_JUMP) && !(m_nOldButtons & IN_JUMP)) {
				pi.grounded = false;
			}
		}

		pi.position = pl->abs_origin();
		pi.angles = engine->GetAngles(pi.slot);
		pi.velocity = pl->abs_velocity();

		pi.oldButtons = m_nOldButtons;

		if (fabsf(*engine->interval_per_tick - 1.0f/60.0f) < 0.00001f) {
			// Back compat - this used to be hardcoded, and maybe the engine's interval
			// could be slightly different to the value we used, leading to desyncs on
			// old scripts.
			pi.ticktime = 1.0f / 60.0f;
		} else {
			pi.ticktime = *engine->interval_per_tick;
		}

		return pi;
	}

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
