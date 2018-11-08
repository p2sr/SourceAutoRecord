#include "TimerRule.hpp"

#include <vector>

#include "Modules/Server.hpp"

#include "TimerAction.hpp"

TimerRule::TimerRule(int gameVersion, const char* categoryName, const char* mapName, const char* entityName,
    _TimerRuleCallback2 callback)
    : madeAction(false)
    , gameVersion(gameVersion)
    , categoryName(categoryName)
    , mapName(mapName)
    , entityName(entityName)
    , callback2(callback)
    , entityPtr(nullptr)
    , hasProps(false)
    , propOffset(0)
    , isActive(false)
{
    TimerRule::list.push_back(this);
}
TimerRule::TimerRule(int gameVersion, const char* categoryName, const char* mapName, const char* entityName,
    _TimerRuleCallback callback, const char* className, const char* propName)
    : TimerRule(gameVersion, categoryName, mapName, entityName, nullptr)
{
    this->className = className;
    this->propName = propName;
    this->callback = callback;
    this->hasProps = true;
}
bool TimerRule::Load()
{
    auto info = server->GetEntityInfoByClassName(this->entityName);
    if (info) {
        this->entityPtr = info->m_pEntity;
    }

    if (this->hasProps) {
        server->GetOffset(this->className, this->propName, this->propOffset);
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
        if (this->hasProps) {
            auto prop = reinterpret_cast<int*>((uintptr_t)this->entityPtr + this->propOffset);
            return this->callback(this->entityPtr, prop);
        }
        return this->callback2(this->entityPtr);
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
