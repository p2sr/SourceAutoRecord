#include "SegmentedTools.hpp"
#include "Session.hpp"

SegmentedTools* segmentedTools;

SegmentedTools::SegmentedTools()
    : waitTick(-1)
    , pendingCommands("")
{
    this->hasLoaded = true;
}

CON_COMMAND(wait, "wait <tick> <commands>. Wait for the amount of tick specified.\n")
{
    segmentedTools->waitTick = std::atoi(args[1]) + session->GetTick();
    segmentedTools->pendingCommands = args[2];
}