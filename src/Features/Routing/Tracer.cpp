#include "Tracer.hpp"

#include <cstdlib>
#include <tuple>

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Command.hpp"

Tracer* tracer;

Tracer::Tracer()
    : traces()
{
    this->hasLoaded = true;
}
TraceResult* Tracer::GetTraceResult(int nSlot)
{
    return &this->traces[nSlot];
}
void Tracer::Start(int nSlot, Vector source)
{
    this->GetTraceResult(nSlot)->source = source;
}
void Tracer::Stop(int nSlot, Vector destination)
{
    this->GetTraceResult(nSlot)->destination = destination;
}
void Tracer::Reset(int nSlot)
{
    auto trace = this->GetTraceResult(nSlot);
    trace->source = Vector();
    trace->destination = Vector();
}
std::tuple<float, float, float> Tracer::CalculateDifferences(const TraceResult* trace)
{
    return std::make_tuple(
        trace->destination.x - trace->source.x,
        trace->destination.y - trace->source.y,
        trace->destination.z - trace->source.z);
}
float Tracer::CalculateLength(const TraceResult* trace, TracerLengthType type)
{
    auto x = trace->destination.x - trace->source.x;
    auto y = trace->destination.y - trace->source.y;
    auto z = trace->destination.z - trace->source.z;
    return (type == TracerLengthType::VEC2)
        ? std::sqrt(x * x + y * y)
        : std::sqrt(x * x + y * y + z * z);
}

// Commands

CON_COMMAND(sar_trace_a, "Saves location A for tracing.\n")
{
    auto player = server->GetPlayer();
    if (player) {
        tracer->Start(GET_SLOT(), server->GetAbsOrigin(player));
        console->Print("Saved location A for tracing!\n");
    }
}
CON_COMMAND(sar_trace_b, "Saves location B for tracing.\n")
{
    auto player = server->GetPlayer();
    if (player) {
        tracer->Stop(GET_SLOT(), server->GetAbsOrigin(player));
        console->Print("Saved location B for tracing!\n");
    }
}
CON_COMMAND(sar_trace_result, "Prints tracing result.\n")
{
    auto result = tracer->GetTraceResult(GET_SLOT());
    auto xyz = tracer->CalculateDifferences(result);
    console->Print("A: %.3f/%.3f/%.3f\n", result->source.x, result->source.y, result->source.z);
    console->Print("B: %.3f/%.3f/%.3f\n", result->destination.x, result->destination.y, result->destination.z);
    console->Print("dX: %.3f\n", std::get<0>(xyz));
    console->Print("dY: %.3f\n", std::get<1>(xyz));
    console->Print("dZ: %.3f\n", std::get<2>(xyz));
    console->Print("dXY: %.3f\n", tracer->CalculateLength(result, TracerLengthType::VEC2));
    console->Print("dXYZ: %.3f\n", tracer->CalculateLength(result, TracerLengthType::VEC3));
}
CON_COMMAND(sar_trace_reset, "Resets tracer.\n")
{
    tracer->Reset(GET_SLOT());
}
