#include "DemoGhostEntity.hpp"

#include "DemoGhostPlayer.hpp"
#include "Features/Hud/Toasts.hpp"
#include "Features/Session.hpp"
#include "Modules/Client.hpp"
#include "NetworkGhostPlayer.hpp"

#include <algorithm>

DemoGhostEntity::DemoGhostEntity(unsigned int ID, std::string name, DataGhost data, std::string currentMap)
	: GhostEntity(ID, name, data, currentMap)
	, currentDemo(0)
	, demoTick(0)
	, nbDemoTicks(0)
	, currentMap("")  //currentMapID(engine->GetMapIndex(currentMap))
	, hasFinished(false)
	, offset(0)
	, isAhead(false) {
}

void DemoGhostEntity::ChangeDemo() {
	this->currentDatas = this->datasByLevel[this->currentDemo].levelDatas;
	this->nbDemoTicks = this->datasByLevel[this->currentDemo].demo.playbackTicks;
	this->currentMap = this->datasByLevel[this->currentDemo].demo.mapName;
	this->sameMap = engine->GetCurrentMapName() == this->currentMap;
	this->isAhead = engine->GetMapIndex(this->currentMap) > engine->GetMapIndex(engine->GetCurrentMapName());
}

void DemoGhostEntity::AddLevelDatas(DemoDatas &datas) {
	this->datasByLevel.push_back(datas);
}

void DemoGhostEntity::SetFirstLevelDatas(DemoDatas &datas) {
	if (this->datasByLevel.size() > 0) {
		this->datasByLevel[0] = datas;
	} else {
		this->datasByLevel.push_back(datas);
	}
}

void DemoGhostEntity::NextDemo() {
	if (++this->currentDemo != this->datasByLevel.size()) {
		this->ChangeDemo();
		this->LevelReset();
		if (ghost_show_advancement.GetBool()) {
			std::string msg = Utils::ssprintf("%s is now on %s", this->name.c_str(), this->currentMap.c_str());
			toastHud.AddToast(GHOST_TOAST_TAG, msg);
		}
	} else {
		this->hasFinished = true;
		this->sameMap = false;  //Make the ghost to disappear
		if (ghost_show_advancement.GetBool()) {
			std::string msg = Utils::ssprintf("%s has finished", this->name.c_str());
			toastHud.AddToast(GHOST_TOAST_TAG, msg);
		}
	}
}

void DemoGhostEntity::UpdateDemoGhost() {
	int gameTick = session->isRunning ? session->GetTick() : this->lastGameTick;
	if (gameTick == this->lastGameTick - 1 && gameTick > 1) {
		// This is truly horrifying. When you console pause, for
		// whatever fucking reason, the game reverts a tick. This
		// *should* detect those cases, with the exception of any type
		// of pause on tick <= 1, where... tbh you're on your own
		--this->demoTick;
	}

	if (this->demoTick > this->nbDemoTicks && demoGhostPlayer.IsFullGame()) {  // if played the whole demo
		this->NextDemo();
	} else if (this->demoTick > this->nbDemoTicks) {  // If played the whole CM demo
		this->DeleteGhost();
	} else if (this->demoTick < this->nbDemoTicks && this->demoTick >= 0) {
		auto data = this->currentDatas.find(this->demoTick);
		if (data != this->currentDatas.end()) {
			this->data = data->second;
		} else {
			// No data for this tick! Chances are it's an alternateticks
			// demo. Try to interpolate the position to smooth movement
			// a bit
			data = this->currentDatas.find(this->demoTick + 1);
			if (data != this->currentDatas.end()) {
				this->oldPos = this->data;
				this->data = data->second;
				Math::Lerp(this->oldPos.position, this->data.position, 0.5, this->data.position);
			}
		}

		if (this->sameMap) {
			if (this->prop_entity == nullptr) {
				this->Spawn();
			}
			this->Display();
		}
	}

	// Thanks to demo delay, this isn't necessarily called once every
	// tick, meaning we can get out of sync! Correct for that by seeing
	// how many ticks have *actually* elapsed.
	if (gameTick > this->lastGameTick) {
		this->demoTick += gameTick - this->lastGameTick;
	} else if (gameTick == this->lastGameTick - 1 && gameTick > 1) {
		// Do nothing - see start of function
	} else if (gameTick < this->lastGameTick) {
		// We've gone through a load; one tick has passed
		++this->demoTick;
	}
	this->lastGameTick = gameTick;
}

void DemoGhostEntity::SetGhostOnFirstMap() {
	this->currentDemo = 0;
	this->demoTick = this->offset;
	this->lastGameTick = session->GetTick();

	this->ChangeDemo();
	this->LevelReset();
}

void DemoGhostEntity::LevelReset() {
	if (this->currentDemo == 0) {
		this->demoTick = this->offset;
	} else {
		this->demoTick = 0;
	}

	if (GhostEntity::ghost_type != GhostType::CIRCLE && GhostEntity::ghost_type != GhostType::PYRAMID) {
		this->DeleteGhost();
	}
}

float DemoGhostEntity::GetTotalTime() {
	return (this->totalTicks + -this->offset) * *engine->interval_per_tick;
}

std::string DemoGhostEntity::GetCurrentMap() {
	return this->currentMap;
}
