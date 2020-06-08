#pragma once
#include "Command.hpp"
#include "GhostEntity.hpp"
#include "Variable.hpp"

#include "Features/Speedrun/SpeedrunTimer.hpp"

class DemoGhostEntity : public GhostEntity {
    friend class DemoGhostPlayer;

public:
    DemoGhostEntity(sf::Uint32 ID, std::string name, DataGhost data, std::string currentMap, std::vector<DataGhost>& datas, size_t nbDemoTicks)
        : GhostEntity(ID, name, DataGhost{ { 0, 0, 0 }, { 0, 0, 0 } }, currentMap)
        , datas(datas)
        , tickCount(0)
        , nbDemoTicks(nbDemoTicks)
    {
    }

    void SetCoordList(std::vector<DataGhost>& datas)
    {
        this->datas = datas;
        this->nbDemoTicks = datas.size();
    }

private:
    int finalTime;
    std::vector<DataGhost> datas;
    int tickCount;
    size_t nbDemoTicks;
};

class DemoGhostPlayer {
private:
    std::vector<DemoGhostEntity> ghostPool;
    bool isPlaying;

public:
public:
    DemoGhostPlayer();

    void SpawnAllGhosts();
    void PauseAllGhosts();
    void DeleteAllGhosts();
    void UpdateGhostsPosition();

    DemoGhostEntity* GetGhostByID(int ID);
    void SetDemoTime(float time);
    void AddGhost(DemoGhostEntity& ghost);
    bool IsPlaying();
};

extern DemoGhostPlayer demoGhostPlayer;

extern Command ghost_set_demo;
extern Command ghost_start;
