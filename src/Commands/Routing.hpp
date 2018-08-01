#pragma once
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"

#include "Features/Routing.hpp"

#include "Command.hpp"

using namespace Routing;

CON_COMMAND(sar_trace_a, "Saves location A for tracing.\n")
{
    Tracer::Start(Client::GetAbsOrigin());
    console->Print("Saved location A for tracing!\n");
}

CON_COMMAND(sar_trace_b, "Saves location B for tracing.\n")
{
    Tracer::Stop(Client::GetAbsOrigin());
    console->Print("Saved location B for tracing!\n");
}

CON_COMMAND(sar_trace_result, "Prints tracing result.\n")
{
    auto xyz = Tracer::GetDifferences();
    console->Print("A: %.3f/%.3f/%.3f\n", Tracer::Source.x, Tracer::Source.y, Tracer::Source.z);
    console->Print("B: %.3f/%.3f/%.3f\n", Tracer::Destination.x, Tracer::Destination.y, Tracer::Destination.z);
    console->Print("dX: %.3f\n", std::get<0>(xyz));
    console->Print("dY: %.3f\n", std::get<1>(xyz));
    console->Print("dZ: %.3f\n", std::get<2>(xyz));
    console->Print("dXY: %.3f\n", Tracer::GetResult(Tracer::ResultType::VEC2));
    console->Print("dXYZ: %.3f\n", Tracer::GetResult(Tracer::ResultType::VEC3));
}

CON_COMMAND(sar_trace_reset, "Resets tracer.\n")
{
    Tracer::Reset();
}