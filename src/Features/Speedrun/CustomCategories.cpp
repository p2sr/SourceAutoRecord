#include "TimerCategory.hpp"
#include "SpeedrunTimer.hpp"
#include "Command.hpp"
#include "SAR.hpp"
#include "Modules/Console.hpp"
#include <map>
#include <string>

struct InputInfo {
    void* entity;
    std::string input;
};

static std::map<const char*, TimerCategory*> customCategories;
static std::map<std::string*, std::string> customRuleInputs;
static std::vector<InputInfo> inputInfos;

void TickCustomCategories()
{
    // Runs at start of tick - empty list ready
    inputInfos.clear();
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
    const char* inputName = customRuleInputs[(std::string*)user].c_str();
    for (auto& inp : inputInfos) {
        if (inp.entity == entity && !strcmp(inputName, inp.input.c_str())) {
            return TimerAction::Start;
        }
    }
    return TimerAction::DoNothing;
}

static TimerAction customRuleEndCallback(void* entity, void* user)
{
    const char* inputName = customRuleInputs[(std::string*)user].c_str();
    for (auto& inp : inputInfos) {
        if (inp.entity == entity && !strcmp(inputName, inp.input.c_str())) {
            return TimerAction::End;
        }
    }
    return TimerAction::DoNothing;
}

CON_COMMAND(sar_speedrun_category_create, "sar_speedrun_category_create <name> <map> <entity> <input> <map> <entity> <input> - creates a custom category with the given name and start/end rules.\n")
{
    if (args.ArgC() != 8) {
        console->Print(sar_speedrun_category_create.ThisPtr()->m_pszHelpString);
        return;
    }

    char* catName = (char*)malloc(strlen(args[1]) + 1);
    strcpy(catName, args[1]);

    if (customCategories.find(catName) != customCategories.end()) {
        console->Print("Category '%s' already exists!\n", catName);
        return;
    }

    std::vector<TimerRule*> rules;

    for (int i = 2; i < 8; i += 3) { // Loops twice
        const char *map = args[i], *entityName = args[i+1], *input = args[i+2];

        std::string* ruleName = new std::string(catName);

        if (i == 2) *ruleName += "_start";
        else *ruleName += "_end";

        customRuleInputs[ruleName] = std::string(input);

        _TimerRuleCallback3 callback = i == 2 ? &customRuleStartCallback : &customRuleEndCallback;
        rules.push_back(new TimerRule(ruleName->c_str(), map, entityName, ruleName, callback));
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

    const char* catName = args[1];

    const char* key = nullptr;

    for (auto it = customCategories.begin(); it != customCategories.end(); ++it) {
        if (!strcmp(it->first, catName)) {
            key = it->first;
            break;
        }
    }

    if (!key) {
        console->Print("Custom category '%s' does not exist!\n", catName);
        return;
    }

    TimerCategory* cat = customCategories[key];

    TimerCategory* cur = speedrun->GetCategory();
    if (cur == cat) {
        console->Print("Cannot delete the active category!\n");
        return;
    }

    for (TimerRule* r : cat->rules) {
        std::string* ruleName = (std::string*)r->user;
        customRuleInputs.erase(ruleName);
        delete ruleName;
    }

    customCategories.erase(catName);
    delete cat;

		console->Print("Successfully deleted category '%s'!\n", catName);
}
