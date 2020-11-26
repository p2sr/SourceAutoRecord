#pragma once
#include "GhostEntity.hpp"

#include "Demo.hpp"

struct DemoDatas {
    std::vector<DataGhost> levelDatas;
    Demo demo;
};

class DemoGhostEntity : public GhostEntity {

private:
    std::vector<DataGhost> currentDatas;
    int currentMapID;
    int tick;
    int demoTick;
    size_t nbDemoTicks;
    int demoPlaybackTicks;
    unsigned int currentDemo;

public:
    std::vector<DemoDatas> datasByLevel;
    int totalTicks;
    std::string lastLevel;
    bool hasFinished;
    int offset;
    bool isAhead;

public:
    DemoGhostEntity(unsigned int ID, std::string name, DataGhost data, std::string currentMap);
    void ChangeLevel(const std::string& mapName);
    void ChangeDemo(const unsigned int demoID);
    //Add demo for full game ghost
    void AddLevelDatas(DemoDatas& datas);
    void SetFirstLevelDatas(DemoDatas& datas);
    //Setup the ghost in order to play next demo
    void NextDemo();
    //Update position of the ghost
    void UpdateDemoGhost();
    //Make the ghost ready for FullGame
    void SetGhostOnFirstMap();
    void Reset();
    int GetTotalTime();
};
