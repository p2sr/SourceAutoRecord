#pragma once
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"

#include "Features/Routing.hpp"

using namespace Routing;

namespace Callbacks {

// Distance calculator
void SaveTracerA()
{
    Tracer::Start(Client::GetAbsOrigin());
    Console::Print("Saved location A for tracing!\n");
}
void SaveTracerB()
{
    Tracer::Stop(Client::GetAbsOrigin());
    Console::Print("Saved location B for tracing!\n");
}
void PrintTracerResult()
{
    auto xyz = Tracer::GetDifferences();
    Console::Print("A: %.3f/%.3f/%.3f\n", Tracer::Source.x, Tracer::Source.y, Tracer::Source.z);
    Console::Print("B: %.3f/%.3f/%.3f\n", Tracer::Destination.x, Tracer::Destination.y, Tracer::Destination.z);
    Console::Print("X-Distance: %.3f\n", std::get<0>(xyz));
    Console::Print("Y-Distance: %.3f\n", std::get<1>(xyz));
    Console::Print("Z-Distance: %.3f\n", std::get<2>(xyz));
    Console::Print("Total Distance: %.3f\n", Tracer::GetResult());
}
void ResetTracer()
{
    Tracer::Reset();
}
// Print latest velocity peak
void PrintVelocityPeak()
{
    Console::Print("Maximum Velocity: %.3f\n", Velocity::Peak);
}
void ResetVelocityPeak()
{
    Velocity::Reset();
}
}