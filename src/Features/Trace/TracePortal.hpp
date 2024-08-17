#pragma once

#include <Utils/SDK/Math.hpp>
#include <Utils/SDK/Color.hpp>

#include <vector>

namespace Trace {
	struct PortalLocation {
		Vector position;
		QAngle angles;
		bool is_primary;
		bool is_coop;
		bool is_atlas;

		void Draw() const;
		Color GetColor() const;
		Vector GetDrawOrigin() const;
		Matrix GetDrawRotationMatrix() const;
	};

	struct PortalsList {
		std::vector<Trace::PortalLocation> locations;

		void Draw() const;
		static PortalsList FetchCurrentLocations();
	};
}