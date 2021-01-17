#include "DemoGhostEntity.hpp"

#include "DemoGhostPlayer.hpp"
#include "NetworkGhostPlayer.hpp"

#include "Modules/Client.hpp"

#include <algorithm>

DemoGhostEntity::DemoGhostEntity(unsigned int ID, std::string name, DataGhost data, std::string currentMap)
    : GhostEntity(ID, name, data, currentMap)
    , currentMapID(engine->GetMapIndex(currentMap))
    , tick(0)
    , demoTick(0)
    , nbDemoTicks(0)
    , demoPlaybackTicks(0)
    , currentDemo(0)
    , hasFinished(false)
    , offset(0)
    , isAhead(false)
{
}

void DemoGhostEntity::ChangeLevel(const std::string& mapName)
{
    const auto data_ID = std::distance(this->datasByLevel.begin(), std::find_if(this->datasByLevel.begin(), this->datasByLevel.end(), [&mapName](const DemoDatas& d) { return d.demo.mapName == mapName; }));
    this->ChangeDemo(data_ID);
}

void DemoGhostEntity::ChangeDemo(const unsigned int demoID)
{
    this->currentDemo = demoID;
    this->currentDatas = this->datasByLevel[demoID].levelDatas;
    this->nbDemoTicks = this->datasByLevel[demoID].levelDatas.size();
    this->demoPlaybackTicks = this->datasByLevel[demoID].demo.playbackTicks;
    this->currentMap = this->datasByLevel[demoID].demo.mapName;
    this->currentMapID = engine->GetMapIndex(this->currentMap);
    this->sameMap = engine->m_szLevelName == this->currentMap;
    this->isAhead = this->currentMapID > engine->GetMapIndex(engine->m_szLevelName);
}

void DemoGhostEntity::AddLevelDatas(DemoDatas& datas)
{
    this->datasByLevel.push_back(datas);
}

void DemoGhostEntity::SetFirstLevelDatas(DemoDatas& datas)
{
    if (this->datasByLevel.size() > 0) {
        this->datasByLevel[0] = datas;
    } else {
        this->datasByLevel.push_back(datas);
    }
}

void DemoGhostEntity::NextDemo()
{
    if (++this->currentMapID != this->datasByLevel.size()) {
        this->ChangeDemo(++this->currentDemo);
        this->Reset();
        if (ghost_show_advancement.GetBool()) {
            client->Chat(TextColor::GREEN, "%s is now on %s", this->name.c_str(), this->currentMap.c_str());
        }
    } else {
        this->hasFinished = true;
        this->sameMap = false; //Make the ghost to disapear
        if (ghost_show_advancement.GetBool()) {
            client->Chat(TextColor::GREEN, "%s has finished", this->name.c_str());
        }
    }
}

void DemoGhostEntity::UpdateDemoGhost()
{
    if (++this->tick % 2 == 0) {
        if (++this->demoTick > this->nbDemoTicks && demoGhostPlayer.IsFullGame() && this->tick >= this->demoPlaybackTicks) { //Next demo
            this->NextDemo();
        } else if (this->demoTick < this->nbDemoTicks && this->demoTick >= 0) {
            this->data = this->currentDatas[this->demoTick];
            if (this->sameMap) {
                if (this->entity == nullptr) {
                    this->Spawn();
                }
                this->Display();
            }
        }
    }
}

void DemoGhostEntity::SetGhostOnFirstMap()
{
    this->ChangeLevel(0);
    this->Reset();
}

void DemoGhostEntity::Reset()
{
    this->tick = 0;
    if (this->currentDemo == 0) {
        this->demoTick = this->offset;
    } else {
        this->demoTick = 0;
    }

    if (GhostEntity::ghost_type) {
        this->DeleteGhost();
    }
}

int DemoGhostEntity::GetTotalTime()
{
    return (this->totalTicks + this->offset) * speedrun->GetIntervalPerTick();
}
