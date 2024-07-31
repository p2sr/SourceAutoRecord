#pragma once
#include "Demo.hpp"
#include "GhostEntity.hpp"

#include <unordered_map>

struct DemoData {
	std::map<int, DataGhost> levelData;
	Demo demo;
};

typedef std::unordered_map<std::string, std::tuple<int, bool>> CustomData;

class DemoGhostEntity : public GhostEntity {
private:
	std::map<int, DataGhost> currentData;  //Pos and ang of ghost
	unsigned int currentDemo;

public:
	int lastGameTick;
	int demoTick;
	size_t nbDemoTicks;
	std::vector<DemoData> dataByLevel;
	std::string currentMap;
	int totalTicks;
	std::string firstLevel;
	std::string lastLevel;
	bool hasFinished;
	int offset;
	bool isAhead;

	CustomData customData;

public:
	DemoGhostEntity(unsigned int ID, std::string name, DataGhost data, std::string currentMap);
	void ChangeDemo();  //Change demo for FullGame ghosts
	//Add demo for full game ghost
	void AddLevelData(DemoData &data);
	void SetFirstLevelData(DemoData &data);
	//Setup the ghost in order to play next demo
	void NextDemo();
	//Update position of the ghost
	void UpdateDemoGhost();
	//Make the ghost ready for whole run (just classic reset for CM)
	void SetGhostOnFirstMap();
	//Make ghost ready for spawn
	void LevelReset();
	float GetTotalTime();
	std::string GetCurrentMap();
};
