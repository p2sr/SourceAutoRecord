#pragma once
#include "Utils.hpp"

namespace Helper
{
	namespace Tracer
	{
		Vector Source;
		Vector Destination;

		void Start(Vector source)
		{
			Source = source;
		}
		void Stop(Vector destination)
		{
			Destination = destination;
		}
		void Reset()
		{
			Source = Vector();
			Destination = Vector();
		}
		float GetDifferenceX()
		{
			return Destination.x - Source.x;
		}
		float GetDifferenceY()
		{
			return Destination.y - Source.y;
		}
		float GetDifferenceZ()
		{
			return Destination.z - Source.z;
		}
		float GetResult()
		{
			auto x = GetDifferenceX();
			auto y = GetDifferenceY();
			auto z = GetDifferenceZ();
			return std::sqrt(x * x + y * y + z * z);
		}
	}
	namespace Velocity
	{
		float Maximum;

		void Save(Vector velocity, bool xyOnly = false)
		{
			auto vel = (xyOnly) ? velocity.Length2D() : velocity.Length();
			if (vel > Maximum) Maximum = vel;
		}
		void Reset()
		{
			Maximum = 0;
		}
	}
}