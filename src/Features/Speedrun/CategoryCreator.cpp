#include <string>
#include "Command.hpp"
#include "Modules/Engine.hpp"

static bool g_creatorActive;
static std::string g_creatorCategory;
static std::string g_creatorDefaults;

CON_COMMAND(sar_speedrun_cc_start, "sar_speedrun_cc_start <category name> [default options]... - start the category creator.\n")
{
    if (args.ArgC() < 2) {
        console->Print(sar_speedrun_cc_start.ThisPtr()->m_pszHelpString);
        return;
    }

    if (g_creatorActive) {
        console->Print("Category creation already in progress!\n");
        return;
    }

    if (!strcmp(args[1], "")) {
        console->Print("Category name cannot be empty\n");
        return;
    }

    g_creatorActive = true;
    g_creatorCategory = args[1];
    g_creatorDefaults = "";
    for (int i = 2; i < args.ArgC(); ++i) {
        g_creatorDefaults += std::string("\"") + args[i] + "\" ";
    }

    std::string cmd = "sar_speedrun_category_create \"" + g_creatorCategory + "\"";
    engine->ExecuteCommand(cmd.c_str());
}

CON_COMMAND(sar_speedrun_cc_rule, "sar_speedrun_rule <rule name> <rule type> [options]... - add a rule to the category currently being created.\n")
{
    if (args.ArgC() < 3) {
        console->Print(sar_speedrun_cc_rule.ThisPtr()->m_pszHelpString);
        return;
    }

    if (!g_creatorActive) {
        console->Print("No category creation in progress!\n");
        return;
    }

    std::string options = g_creatorDefaults;
    for (int i = 3; i < args.ArgC(); ++i) {
        options += std::string("\"") + args[i] + "\" ";
    }

    std::string ruleName = g_creatorCategory + " - " + args[1];

    std::string cmd = "sar_speedrun_rule_create \"" + ruleName + "\" \"" + args[2] + "\" " + options;
    engine->ExecuteCommand(cmd.c_str());

    cmd = "sar_speedrun_category_add_rule \"" + g_creatorCategory + "\" \"" + ruleName + "\"";
    engine->ExecuteCommand(cmd.c_str());
}

CON_COMMAND(sar_speedrun_cc_finish, "sar_speedrun_cc_finish - finish the category creator.\n")
{
    if (!g_creatorActive) {
        console->Print("No category creation in progress!\n");
        return;
    }

    console->Print("Created category '%s'\n", g_creatorCategory.c_str());

    g_creatorActive = false;
    g_creatorCategory = "";
    g_creatorDefaults = "";
}
