#include "DemoGhostEntity.hpp"

#include "DemoGhostPlayer.hpp"
#include "NetworkGhostPlayer.hpp"

#include "Modules/Client.hpp"

#include "Features/Session.hpp"

#include <algorithm>

DemoGhostEntity::DemoGhostEntity(unsigned int ID, std::string name, DataGhost data, std::string currentMap)
    : GhostEntity(ID, name, data, currentMap)
    , currentDemo(0)
    , demoTick(0)
    , nbDemoTicks(0)
    , currentMap("") //currentMapID(engine->GetMapIndex(currentMap))
    , hasFinished(false)
    , offset(0)
    , isAhead(false)
{
}

void DemoGhostEntity::ChangeDemo()
{
    this->currentDatas = this->datasByLevel[this->currentDemo].levelDatas;
    this->nbDemoTicks = this->datasByLevel[this->currentDemo].levelDatas.size();
    this->currentMap = this->datasByLevel[this->currentDemo].demo.mapName;
    this->sameMap = engine->GetCurrentMapName() == this->currentMap;
    this->isAhead = engine->GetMapIndex(this->currentMap) > engine->GetMapIndex(engine->GetCurrentMapName());
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
    if (++this->currentDemo != this->datasByLevel.size()) {
        this->ChangeDemo();
        this->LevelReset();
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
    if (session->GetTick() % 2 == 0)
        ++this->demoTick;

    if (this->demoTick > this->nbDemoTicks && demoGhostPlayer.IsFullGame()) { // if played the whole demo
        this->NextDemo();
    } else if (this->demoTick > this->nbDemoTicks) { // If played the whole CM demo
        this->DeleteGhost();
    } else if (this->demoTick < this->nbDemoTicks && this->demoTick >= 0) {
        if (session->GetTick() % 2 == 1) { // Smooth the ghost movement
            this->oldPos = this->data;
            this->data = this->currentDatas[this->demoTick+1];
            Math::Lerp(this->oldPos.position, this->data.position, 0.5, this->data.position);
        } else {
            this->data = this->currentDatas[this->demoTick];
        }

        if (this->sameMap) {
            if (this->prop_entity == nullptr) {
                this->Spawn();
            }
            this->Display();
        }
    }
}

void DemoGhostEntity::SetGhostOnFirstMap()
{
    this->currentDemo = 0;
    this->demoTick = this->offset;

    this->ChangeDemo();
    this->LevelReset();
}

void DemoGhostEntity::LevelReset()
{
    if (this->currentDemo == 0) {
        this->demoTick = this->offset;
    } else {
        this->demoTick = 0;
    }

    if (GhostEntity::ghost_type != GhostType::CIRCLE && GhostEntity::ghost_type != GhostType::PYRAMID) {
        this->DeleteGhost();
    }
}

float DemoGhostEntity::GetTotalTime()
{
    return (this->totalTicks + -this->offset) * speedrun->GetIntervalPerTick();
}

std::string DemoGhostEntity::GetCurrentMap()
{
    return this->currentMap;
}
