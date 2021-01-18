#include "SegmentedTools.hpp"
#include "Session.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"


Variable wait_mode("wait_mode", "0", "When the pending commands should be executed. 0 is absolute, 1 is relative to when you entered the wait command.\n");

SegmentedTools* segmentedTools;

SegmentedTools::SegmentedTools()
    : waitTick(-1)
    , pendingCommands("")
{
    this->hasLoaded = true;
}

CON_COMMAND(wait, "wait <tick> <commands>. Wait for the amount of tick specified.\n")
{
    if (args.ArgC() <= 2) {
        return console->Print(wait.ThisPtr()->m_pszHelpString);
    }

    segmentedTools->waitTick = wait_mode.GetBool() ? std::atoi(args[1]) + session->GetTick() : std::atoi(args[1]);
    segmentedTools->pendingCommands = args[2];
    engine->hasWaited = false;
}