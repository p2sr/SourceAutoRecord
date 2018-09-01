#include "Tracer.hpp"

#include <cstdlib>

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"

#include "Features/Routing/Tracer.hpp"

#include "Command.hpp"

CON_COMMAND(sar_trace_a, "Saves location A for tracing.\n")
{
    tracer->Start(client->GetAbsOrigin());
    console->Print("Saved location A for tracing!\n");
}

CON_COMMAND(sar_trace_b, "Saves location B for tracing.\n")
{
    tracer->Stop(client->GetAbsOrigin());
    console->Print("Saved location B for tracing!\n");
}

CON_COMMAND(sar_trace_result, "Prints tracing result.\n")
{
    auto xyz = tracer->GetDifferences();
    console->Print("A: %.3f/%.3f/%.3f\n", tracer->source.x, tracer->source.y, tracer->source.z);
    console->Print("B: %.3f/%.3f/%.3f\n", tracer->destination.x, tracer->destination.y, tracer->destination.z);
    console->Print("dX: %.3f\n", std::get<0>(xyz));
    console->Print("dY: %.3f\n", std::get<1>(xyz));
    console->Print("dZ: %.3f\n", std::get<2>(xyz));
    console->Print("dXY: %.3f\n", tracer->GetResult(TracerResultType::VEC2));
    console->Print("dXYZ: %.3f\n", tracer->GetResult(TracerResultType::VEC3));
}

CON_COMMAND(sar_trace_reset, "Resets tracer.\n")
{
    tracer->Reset();
}
