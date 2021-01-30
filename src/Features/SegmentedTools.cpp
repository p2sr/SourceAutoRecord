#include "SegmentedTools.hpp"
#include "Session.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

Variable wait_persist_across_loads("wait_persist_across_loads", "0", 0, 1, "Whether pending commands should be carried across loads (1) or just be dropped (0).\n");

Variable wait_mode("wait_mode", "0", "When the pending commands should be executed. 0 is absolute, 1 is relative to when you entered the wait command.\n");

SegmentedTools* segmentedTools;

SegmentedTools::SegmentedTools()
    : waitTick(-1)
    , pendingCommands("")
{
    this->hasLoaded = true;
}

// mlugg 2021-01-11: DO NOT USE CON_COMMAND FOR THIS. That macro creates
// a global named 'wait', which on Linux, conflicts with
// 'pid_t wait(int *wstatus)' from stdlib. This results in very bad
// things happening on initialization; C++ gets them mixed up somehow
// and tries to send the function pointer as the Command constructor's
// 'this', with predictably disastrous results.
void wait_callback(const CCommand& args)
{
    if (args.ArgC() <= 2) {
        return console->Print(waitCmd.ThisPtr()->m_pszHelpString);
    }

    if (!sv_cheats.GetBool()) {
        console->Print("\"wait\" needs sv_cheats 1.\n");
        return;
    }

    segmentedTools->waitTick = wait_mode.GetBool() ? std::atoi(args[1]) + session->GetTick() : std::atoi(args[1]);
    segmentedTools->pendingCommands = args[2];
    engine->hasWaited = false;
}
Command waitCmd = Command("wait", wait_callback, "wait <tick> <commands>. Wait for the amount of tick specified.\n");
