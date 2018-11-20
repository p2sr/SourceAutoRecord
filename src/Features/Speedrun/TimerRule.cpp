#include "TimerRule.hpp"

#include <vector>

#include "Features/EntityList.hpp"
#include "Features/OffsetFinder.hpp"

#include "Modules/Console.hpp"
#include "Modules/Server.hpp"

#include "TimerAction.hpp"

TimerRule::TimerRule(int gameVersion, const char* categoryName, const char* mapName, const char* entityName,
    _TimerRuleCallback0 callback, SearchMode searchMode)
    : madeAction(false)
    , gameVersion(gameVersion)
    , categoryName(categoryName)
    , mapName(mapName)
    , entityName(entityName)
    , callback0(callback)
    , entityPtr(nullptr)
    , callbackType(0)
    , propOffset(0)
    , isActive(false)
    , searchMode(searchMode)
{
    TimerRule::list.push_back(this);
}
TimerRule::TimerRule(int gameVersion, const char* categoryName, const char* mapName, const char* entityName,
    _TimerRuleCallback1 callback, const char* className, const char* propName, SearchMode searchMode)
    : TimerRule(gameVersion, categoryName, mapName, entityName, nullptr, searchMode)
{
    this->className = className;
    this->propName = propName;
    this->callback1 = callback;
    this->callbackType = 1;
}
TimerRule::TimerRule(int gameVersion, const char* categoryName, const char* mapName, const char* entityName,
    _TimerRuleCallback2 callback, const char* className, const char* propName, SearchMode searchMode)
    : TimerRule(gameVersion, categoryName, mapName, entityName, nullptr, searchMode)
{
    this->className = className;
    this->propName = propName;
    this->callback2 = callback;
    this->callbackType = 2;
}
bool TimerRule::Load()
{
    auto info = (this->searchMode == SearchMode::Classes)
        ? entityList->GetEntityInfoByClassName(this->entityName)
        : entityList->GetEntityInfoByName(this->entityName);

    if (info) {
        this->entityPtr = info->m_pEntity;
    } else {
        if (this->searchMode == SearchMode::Classes) {
            console->Warning("There isn't any entity with the class name: %s\n", this->entityName);
        } else {
            console->Warning("There is no entity with the name: %s\n", this->entityName);
        }
    }

    if (this->callbackType != 0) {
        offsetFinder->ServerSide(this->className, this->propName, &this->propOffset);
        return this->isActive = (this->entityPtr != nullptr && this->propOffset != 0);
    }

    return this->isActive = this->entityPtr != nullptr;
}
void TimerRule::Unload()
{
    this->entityPtr = nullptr;
    this->propOffset = 0;
    this->isActive = false;
}
TimerAction TimerRule::Dispatch()
{
    if (this->isActive) {
        if (this->callbackType == 1) {
            auto prop = reinterpret_cast<int*>((uintptr_t)this->entityPtr + this->propOffset);
            return this->callback1(this->entityPtr, prop);
        } else if (this->callbackType == 2) {
            return this->callback2(this->entityPtr, this->propOffset);
        }
        return this->callback0(this->entityPtr);
    }

    return TimerAction::DoNothing;
}
int TimerRule::FilterByGame(Game* game)
{
    auto count = 0;
    for (auto&& rule = TimerRule::list.begin(); rule != TimerRule::list.end();) {
        if ((*rule)->gameVersion != game->version) {
            rule = TimerRule::list.erase(rule);
        } else {
            ++rule;
            ++count;
        }
    }

    return count;
}
void TimerRule::ResetAll()
{
    for (auto& rule : TimerRule::list) {
        rule->madeAction = false;
    }
}

std::vector<TimerRule*> TimerRule::list;
