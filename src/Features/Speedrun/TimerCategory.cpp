#include "TimerCategory.hpp"

#include <vector>

#include "Features/EntityList.hpp"
#include "Features/OffsetFinder.hpp"

#include "Modules/Console.hpp"
#include "Modules/Server.hpp"

#include "TimerAction.hpp"

TimerCategory::TimerCategory(int gameVersion, const char* name, std::vector<TimerRule*> rules)
    : gameVersion(gameVersion)
    , name(name)
    , rules(rules)
{
    TimerCategory::list.push_back(this);
}
int TimerCategory::FilterByGame(Game* game)
{
    auto count = 0;
    for (auto&& rule = TimerCategory::list.begin(); rule != TimerCategory::list.end();) {
        if (!game->Is((*rule)->gameVersion)) {
            rule = TimerCategory::list.erase(rule);
        } else {
            ++rule;
            ++count;
        }
    }

    return count;
}
void TimerCategory::ResetAll()
{
    for (auto& category : TimerCategory::list) {
        for (auto& rule : category->rules) {
            rule->madeAction = false;
        }
    }
}

std::vector<TimerCategory*> TimerCategory::list;
