#include "SegmentedTools.hpp"
#include "Session.hpp"

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
    segmentedTools->waitTick = std::atoi(args[1]) + session->GetTick();
    segmentedTools->pendingCommands = args[2];
}
Command waitCmd = Command("wait", wait_callback, "wait <tick> <commands>. Wait for the amount of tick specified.\n");
