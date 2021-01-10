#pragma once
#include "GhostEntity.hpp"

#include "Demo.hpp"

#include "SFML/Network.hpp"

struct DemoDatas {
    std::vector<DataGhost> levelDatas;
    Demo demo;
};

class DemoGhostEntity : public GhostEntity {

private:
    std::vector<DataGhost> currentDatas; //Pos and ang of ghost
    int demoTick;
    size_t nbDemoTicks;
    unsigned int currentDemo;

public:
    std::vector<DemoDatas> datasByLevel;
    std::string currentMap;
    int totalTicks;
    std::string firstLevel;
    std::string lastLevel;
    bool hasFinished;
    int offset;
    bool isAhead;

public:
    DemoGhostEntity(sf::Uint32 ID, std::string name, DataGhost data, std::string currentMap);
    void ChangeDemo(); //Change demo for FullGame ghosts
    //Add demo for full game ghost
    void AddLevelDatas(DemoDatas& datas);
    void SetFirstLevelDatas(DemoDatas& datas);
    //Setup the ghost in order to play next demo
    void NextDemo();
    //Update position of the ghost
    void UpdateDemoGhost();
    //Make the ghost ready for whole run (just classic reset for CM)
    void SetGhostOnFirstMap();
    //Make ghost ready for spawn
    void LevelReset();
    int GetTotalTime();
    std::string GetCurrentMap();
};
