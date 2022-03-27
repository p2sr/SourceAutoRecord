#pragma once
#include "Command.hpp"
#include "Demo.hpp"
#include "DemoGhostEntity.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/Hud/Toasts.hpp"
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
	int followID;

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
	std::vector<DemoGhostEntity>& GetAllGhosts();

	DemoGhostEntity *GetGhostByID(unsigned ID);

	bool SetupGhostFromDemo(const std::string &demo_path, const unsigned int ghost_ID, bool fullGame);
	void AddGhost(DemoGhostEntity &ghost);
	bool IsPlaying();
	bool IsFullGame();

	void PrintRecap();

	template <typename... Ts>
	void TestInputRule(Ts... args);
	std::string CustomDataToString(const char *entName, const char *className, const char *inputName, const char *parameter, std::optional<int> activatorSlot);
	std::string CustomDataToString(Vector pos, std::optional<int> slot, PortalColor portal);
	std::string CustomDataToString(std::optional<int> slot);
};

extern DemoGhostPlayer demoGhostPlayer;

template <typename... Ts>
void DemoGhostPlayer::TestInputRule(Ts... args) {
	if (!demoGhostPlayer.IsPlaying()) return;

	std::string text;
	for (auto &ghost : demoGhostPlayer.GetAllGhosts()) {
		std::string str = this->CustomDataToString(args...);
		if (auto it{ghost.customDatas.find(str)}; it != ghost.customDatas.end()) {
			std::get<1>(it->second) = true;
			int tick = std::get<0>(it->second);

			if (tick > SpeedrunTimer::GetTotalTicks())
				text += Utils::ssprintf("%s -> -%ss ", ghost.name.c_str(), SpeedrunTimer::Format((tick - SpeedrunTimer::GetTotalTicks()) * *engine->interval_per_tick).c_str());
			else
				text += Utils::ssprintf("%s -> +%ss ", ghost.name.c_str(), SpeedrunTimer::Format((SpeedrunTimer::GetTotalTicks() - tick) * *engine->interval_per_tick).c_str());
		}
	}

	if (!text.empty())
		toastHud.AddToast(SPEEDRUN_TOAST_TAG, text);
}

extern Variable ghost_sync;
extern Command ghost_set_demo;
extern Command ghost_set_demos;
extern Command ghost_delete_all;
extern Command ghost_delete_by_ID;
extern Command ghost_recap;
extern Command ghost_start;
extern Command ghost_reset;
extern Command ghost_offset;
