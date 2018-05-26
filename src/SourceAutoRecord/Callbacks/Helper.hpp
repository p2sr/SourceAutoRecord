#pragma once
#include "Modules/Console.hpp"
#include "Modules/Client.hpp"

#include "Features/Helper.hpp"

using namespace Helper;

namespace Callbacks
{
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
		Console::Print("A: %.3f/%.3f/%.3f\n", Tracer::Source.x, Tracer::Source.y, Tracer::Source.z);
		Console::Print("B: %.3f/%.3f/%.3f\n", Tracer::Destination.x, Tracer::Destination.y, Tracer::Destination.z);
		Console::Print("X-Distance: %.3f\n", Tracer::GetDifferenceX());
		Console::Print("Y-Distance: %.3f\n", Tracer::GetDifferenceY());
		Console::Print("Z-Distance: %.3f\n", Tracer::GetDifferenceZ());
		Console::Print("Total Distance: %.3f\n", Tracer::GetResult());
	}
	void ResetTracer()
	{
		Tracer::Reset();
	}
	// Print last maximum velocity
	void PrintMaxVel()
	{
		Console::Print("Maximum Velocity: %.3f\n", Velocity::Maximum);
	}
	void ResetMaxVel()
	{
		Velocity::Reset();
	}
}