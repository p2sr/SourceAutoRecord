#include "Tracer.hpp"

#include <cstdlib>
#include <tuple>

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"

#include "Command.hpp"

Tracer* tracer;

Tracer::Tracer()
    : source()
    , destination()
{
    this->hasLoaded = true;
}
void Tracer::Start(Vector source)
{
    this->source = source;
}
void Tracer::Stop(Vector destination)
{
    this->destination = destination;
}
void Tracer::Reset()
{
    this->source = Vector();
    this->destination = Vector();
}
std::tuple<float, float, float> Tracer::GetDifferences()
{
    return std::make_tuple(
        this->destination.x - this->source.x,
        this->destination.y - this->source.y,
        this->destination.z - this->source.z);
}
float Tracer::GetResult(TracerResultType type)
{
    auto x = this->destination.x - this->source.x;
    auto y = this->destination.y - this->source.y;
    auto z = this->destination.z - this->source.z;
    return (type == TracerResultType::VEC2)
        ? std::sqrt(x * x + y * y)
        : std::sqrt(x * x + y * y + z * z);
}

// Commands

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
