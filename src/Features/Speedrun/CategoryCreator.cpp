#include <string>
#include "Command.hpp"
#include "Modules/Engine.hpp"
#include "CategoryCreator.hpp"
#include "Modules/Server.hpp"

#define MAX_TRACE 65535.0f

#ifdef _WIN32
#    define PLAT_CALL(fn, ...) fn(__VA_ARGS__)
#else
#    define PLAT_CALL(fn, ...) fn(nullptr, __VA_ARGS__)
#endif

static bool g_creatorActive;
static std::string g_creatorCategory;
static std::string g_creatorDefaults;

static int g_placementStage;
static std::string g_placementName;
static std::string g_placementType;
static std::string g_placementOptions;
static Vector g_placementStart;

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

    g_placementStage = 0;
    g_placementName = "";
    g_placementType = "";
    g_placementOptions = "";

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

CON_COMMAND(sar_speedrun_cc_place_start, "sar_speedrun_cc_place_start <rule name> <rule type> [options]... - start placing a trigger-ey rule in the world.\n")
{
    if (args.ArgC() < 3) {
        console->Print(sar_speedrun_cc_place_start.ThisPtr()->m_pszHelpString);
        return;
    }

    if (!g_creatorActive) {
        console->Print("No category creation in progress!\n");
        return;
    }

    if (!sv_cheats.GetBool()) {
        console->Print("Trigger placement cannot occur without sv_cheats!\n");
        return;
    }

    if (g_placementStage != 0) {
        console->Print("Trigger placement already in progress!\n");
        return;
    }

    std::string ruleName = g_creatorCategory + " - " + args[1];
    std::string ruleType = args[2];

    if (ruleType != "portal" && ruleType != "zone") {
        console->Print("'%s' is not a valid trigger-ey rule type!\n", ruleType.c_str());
        return;
    }

    std::string options = "";
    for (int i = 3; i < args.ArgC(); ++i) {
        options += std::string("\"") + args[i] + "\" ";
    }

    g_placementStage = 1;
    g_placementName = ruleName;
    g_placementType = ruleType;
    g_placementOptions = options;
}

CON_COMMAND(sar_speedrun_cc_place, "sar_speedrun_cc_place - place a trigger-ey rule in the world.\n")
{
    if (!g_creatorActive) {
        console->Print("No category creation in progress!\n");
        return;
    }

    if (!sv_cheats.GetBool()) {
        console->Print("Trigger placement cannot occur without sv_cheats!\n");
        return;
    }

    switch (g_placementStage) {
    case 0: {
        console->Print("No trigger placement in progress!\n");
        return;
    }
    case 1: {
        CGameTrace tr;
        engine->TraceFromCamera(MAX_TRACE, tr);

        g_placementStart = tr.endpos;

        g_placementStage = 2;

        break;
    }
    case 2: {
        CGameTrace tr;
        engine->TraceFromCamera(MAX_TRACE, tr);

        Vector start = g_placementStart;
        Vector end = tr.endpos;

        Vector center = (start + end) / 2;

        Vector size = start - end;
        size.x = abs(size.x);
        size.y = abs(size.y);
        size.z = abs(size.z);

        if (g_placementType == "portal") {
            // For portal rules, we expand the size by a unit in each
            // direction just to make sure that triggers on flat
            // surfaces work okay.
            size.x += 1;
            size.y += 1;
            size.z += 1;
        }

        float angle = 0;

        std::string cmd = Utils::ssprintf("sar_speedrun_rule_create \"%s\" \"%s\" %s pos=%f,%f,%f size=%f,%f,%f angle=%f %s", g_placementName.c_str(), g_placementType.c_str(), g_creatorDefaults.c_str(), center.x, center.y, center.z, size.x, size.y, size.z, angle, g_placementOptions.c_str());
        engine->ExecuteCommand(cmd.c_str());
        cmd = Utils::ssprintf("sar_speedrun_category_add_rule \"%s\" \"%s\"", g_creatorCategory.c_str(), g_placementName.c_str());
        engine->ExecuteCommand(cmd.c_str());

        g_placementStage = 0;

        break;
    }
    }
}

CON_COMMAND(sar_speedrun_cc_finish, "sar_speedrun_cc_finish - finish the category creator.\n")
{
    if (!g_creatorActive) {
        console->Print("No category creation in progress!\n");
        return;
    }

    if (g_placementStage != 0) {
        console->Print("A trigger placement still in progress!\n");
        return;
    }

    console->Print("Created category '%s'\n", g_creatorCategory.c_str());

    g_creatorActive = false;
    g_creatorCategory = "";
    g_creatorDefaults = "";

    g_placementStage = 0;
    g_placementName = "";
    g_placementType = "";
    g_placementOptions = "";
}

void SpeedrunTimer::DrawCategoryCreatorPlacement()
{
    if (!sv_cheats.GetBool()) {
        return;
    }

    if (g_placementStage == 2) {
        CGameTrace tr;
        engine->TraceFromCamera(MAX_TRACE, tr);

        PLAT_CALL(
            engine->AddBoxOverlay,
            Vector{ 0, 0, 0 },
            g_placementStart,
            tr.endpos,
            { 0, 0, 0 },
            255, 0, 0, 0,
            2 * *engine->interval_per_tick
        );
    }
}
