#pragma once

#include <Utils/SDK/Math.hpp>

#include <vector>

namespace Trace {
	struct PortalLocation {
		Vector position;
		QAngle angles;
		bool is_primary;
		bool is_coop;
		bool is_atlas;
	};

	struct PortalsList {
		std::vector<Trace::PortalLocation> locations;
	};

	// Construct a list of all portals in the map
	PortalsList FetchCurrentPortalLocations();
	// Draw entire portals list
	void DrawPortals(PortalsList list);
	// Draws given portal data in world
	void DrawPortal(PortalLocation portal);
}