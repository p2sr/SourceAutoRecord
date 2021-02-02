#include "TimerCategory.hpp"
#include "SpeedrunTimer.hpp"
#include "Command.hpp"
#include "SAR.hpp"
#include "Modules/Console.hpp"
#include "Features/Session.hpp"
#include <map>
#include <string>

struct InputInfo {
    void* entity;
    std::string input;
};

static std::map<char*, TimerCategory*> customCategories;
static std::map<char*, char*> customRuleInputs;
static std::vector<InputInfo> inputInfos;

void TickCustomCategories()
{
    // Runs at start of tick - empty list ready
    // This is kinda weird. The session doesn't start til tick 2, but we
    // get events on ticks 0 and 1; therefore, if we clear the events
    // before tick 3, we lose out on inputs that triggered on those
    // first couple ticks, which is bad
    if (session->GetTick() >= 3) inputInfos.clear();
}

void CheckCustomCategoryRules(void* entity, const char* input)
{
    inputInfos.push_back({
        entity,
        input,
    });
}

static TimerAction customRuleStartCallback(void* entity, void* user)
{
    const char* inputName = customRuleInputs[(char*)user];
    for (auto& inp : inputInfos) {
        if (inp.entity == entity && !strcmp(inputName, inp.input.c_str())) {
            return TimerAction::Start;
        }
    }
    return TimerAction::DoNothing;
}

static TimerAction customRuleEndCallback(void* entity, void* user)
{
    const char* inputName = customRuleInputs[(char*)user];
    for (auto& inp : inputInfos) {
        if (inp.entity == entity && !strcmp(inputName, inp.input.c_str())) {
            return TimerAction::End;
        }
    }
    return TimerAction::DoNothing;
}

static char* dupString(const char* str)
{
    char* newStr = (char*)malloc(strlen(str) + 1);
    return strcpy(newStr, str);
}

CON_COMMAND(sar_speedrun_category_create, "sar_speedrun_category_create <name> <map> <entity> <input> <map> <entity> <input> - creates a custom category with the given name and start/end rules.\nEntity names may begin with ! to specify a classname rather than a targetname.\n")
{
    if (args.ArgC() != 8) {
        console->Print(sar_speedrun_category_create.ThisPtr()->m_pszHelpString);
        return;
    }

    for (auto& c : TimerCategory::GetList()) {
        if (!strcmp(c->name, args[1])) {
            console->Print("Category '%s' already exists!\n", args[1]);
            return;
        }
    }

    char* catName = dupString(args[1]);

    std::vector<TimerRule*> rules;

    for (int i = 2; i < 8; i += 3) { // Loops twice
        bool isClass = args[i+1][0] == '!';

        char *map = dupString(args[i]);
        char *entityName = dupString(args[i+1] + (isClass ? 1 : 0));
        char *input = dupString(args[i+2]);

        char* ruleName = (char*)malloc(strlen(catName) + 7);
        strcpy(ruleName, catName);
        strcat(ruleName, i == 2 ? "_start" : "_end");

        customRuleInputs[ruleName] = input;

        _TimerRuleCallback3 callback = i == 2 ? &customRuleStartCallback : &customRuleEndCallback;
        rules.push_back(new TimerRule(ruleName, map, entityName, ruleName, callback, isClass ? SearchMode::Classes : SearchMode::Names));
    }

    customCategories[catName] = new TimerCategory(sar.game->GetVersion(), catName, rules);

    console->Print("Category '%s' sucessfully created!\n", catName);
}

CON_COMMAND(sar_speedrun_category_remove, "sar_speedrun_category_remove <name> - removes a custom category.\n")
{
    if (args.ArgC() != 2) {
        console->Print(sar_speedrun_category_remove.ThisPtr()->m_pszHelpString);
        return;
    }

    const char* catNameGiven = args[1];
    char* catNameKey = nullptr;

    for (auto it = customCategories.begin(); it != customCategories.end(); ++it) {
        if (!strcmp(it->first, catNameGiven)) {
            catNameKey = it->first;
            break;
        }
    }

    if (!catNameKey) {
        console->Print("Custom category '%s' does not exist!\n", catNameGiven);
        return;
    }

    TimerCategory* cat = customCategories[catNameKey];

    TimerCategory* cur = speedrun->GetCategory();
    if (cur == cat) {
        console->Print("Cannot delete the active category!\n");
        return;
    }

    for (TimerRule* r : cat->rules) {
        char* ruleName = (char*)r->user;

        // Free input string
        free(customRuleInputs[ruleName]);
        customRuleInputs.erase(ruleName);

        // Free ruleName string
        free(ruleName);

        // Free mapName string
        free((void*)r->mapName);

        // Free entityName string
        free((void*)r->entityName);

        // Free the rule
        delete r;
    }

    customCategories.erase(catNameKey);
    delete cat;

    console->Print("Successfully deleted category '%s'!\n", catNameGiven);
}
