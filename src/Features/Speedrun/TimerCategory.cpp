#include "TimerCategory.hpp"

#include <vector>

#include "Features/EntityList.hpp"
#include "Features/OffsetFinder.hpp"

#include "Modules/Console.hpp"
#include "Modules/Server.hpp"

#include "TimerAction.hpp"

std::vector<TimerCategory*>& TimerCategory::GetList()
{
    static std::vector<TimerCategory*> list;
    return list;
}

TimerCategory::TimerCategory(int gameVersion, const char* name, std::vector<TimerRule*> rules)
    : gameVersion(gameVersion)
    , name(name)
    , rules(rules)
{
    TimerCategory::GetList().push_back(this);
}
TimerCategory::~TimerCategory()
{
    std::vector<TimerCategory*>& cats = TimerCategory::GetList();
    for (size_t i = 0; i < cats.size(); ++i) {
        if (cats[i] == this) {
            cats.erase(cats.begin() + i);
            break;
        }
    }
}
int TimerCategory::FilterByGame(Game* game)
{
    auto count = 0;
    for (auto&& rule = TimerCategory::GetList().begin(); rule != TimerCategory::GetList().end();) {
        if (!game->Is((*rule)->gameVersion)) {
            rule = TimerCategory::GetList().erase(rule);
        } else {
            ++rule;
            ++count;
        }
    }

    return count;
}
void TimerCategory::ResetAll()
{
    for (auto& category : TimerCategory::GetList()) {
        for (auto& rule : category->rules) {
            rule->madeAction = false;
        }
    }
}
