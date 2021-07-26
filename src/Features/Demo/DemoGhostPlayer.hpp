#pragma once
#include "Command.hpp"
#include "Demo.hpp"
#include "DemoGhostEntity.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "GhostEntity.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Variable.hpp"

#include <algorithm>

class DemoGhostPlayer {
private:
	std::vector<DemoGhostEntity> ghostPool;
	bool isPlaying;
	int currentTick;

public:
	bool isFullGame;
	int nbDemos;

public:
	DemoGhostPlayer();

	void SpawnAllGhosts();
	void StartAllGhost();
	void ResetAllGhosts();
	void PauseAllGhosts();
	void ResumeAllGhosts();
	void DeleteAllGhosts();
	void DeleteAllGhostModels();
	void DeleteGhostsByID(const unsigned int ID);
	void UpdateGhostsPosition();
	void UpdateGhostsSameMap();
	void UpdateGhostsModel(const std::string model);
	void Sync();

	DemoGhostEntity *GetGhostByID(int ID);

	bool SetupGhostFromDemo(const std::string &demo_path, const unsigned int ghost_ID, bool fullGame);
	void AddGhost(DemoGhostEntity &ghost);
	bool IsPlaying();
	bool IsFullGame();

	void PrintRecap();
	void DrawNames(HudContext *ctx);
};

extern DemoGhostPlayer demoGhostPlayer;

extern Variable ghost_sync;
extern Command ghost_set_demo;
extern Command ghost_set_demos;
extern Command ghost_delete_all;
extern Command ghost_delete_by_ID;
extern Command ghost_recap;
extern Command ghost_start;
extern Command ghost_reset;
extern Command ghost_offset;
