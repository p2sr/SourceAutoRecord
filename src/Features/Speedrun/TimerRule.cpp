#include "TimerRule.hpp"

#include <vector>

#include "Features/EntityList.hpp"
#include "Features/OffsetFinder.hpp"

#include "Modules/Console.hpp"
#include "Modules/Server.hpp"

#include "TimerAction.hpp"

TimerRule::TimerRule(const char* name, const char* mapName, const char* entityName,
    _TimerRuleCallback0 callback, SearchMode searchMode)
    : madeAction(false)
    , name(name)
    , mapName(mapName)
    , entityName(entityName)
    , callback0(callback)
    , entityPtr(nullptr)
    , callbackType(0)
    , propOffset(0)
    , isActive(false)
    , searchMode(searchMode)
{
}
TimerRule::TimerRule(const char* name, const char* mapName, const char* entityName,
    _TimerRuleCallback1 callback, const char* className, const char* propName, SearchMode searchMode)
    : TimerRule(name, mapName, entityName, nullptr, searchMode)
{
    this->className = className;
    this->propName = propName;
    this->callback1 = callback;
    this->callbackType = 1;
}
TimerRule::TimerRule(const char* name, const char* mapName, const char* entityName,
    _TimerRuleCallback2 callback, const char* className, const char* propName, SearchMode searchMode)
    : TimerRule(name, mapName, entityName, nullptr, searchMode)
{
    this->className = className;
    this->propName = propName;
    this->callback2 = callback;
    this->callbackType = 2;
}
TimerRule::TimerRule(const char* name, const char* mapName, const char* entityName, void* user,
    _TimerRuleCallback3 callback, SearchMode searchMode)
    : TimerRule(name, mapName, entityName, nullptr, searchMode)
{
    this->callback3 = callback;
    this->callbackType = 3;
    this->user = user;
}
bool TimerRule::Load()
{
    if (this->IsEmpty()) {
        return this->isActive = true;
    }

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

    if (this->callbackType != 0 && this->callbackType != 3) {
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

        if (this->callbackType == 3) {
            return this->callback3(this->entityPtr, this->user);
        } else {
            return this->callback0(this->entityPtr);
        }
    }

    return TimerAction::DoNothing;
}
bool TimerRule::IsEmpty()
{
    return this->mapName == nullptr || this->entityName == nullptr;
}
