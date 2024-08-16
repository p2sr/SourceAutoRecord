#pragma once

#include <Utils/SDK/Math.hpp>
#include <vector>

namespace Trace {
	struct VphysHitbox {
		std::vector<Vector> verts;
	};

	struct BoundingHitbox {
		Vector mins, maxs;
		Vector position;
		QAngle angles;
	};

	struct HitboxList {
		std::vector<VphysHitbox> vphys_hitboxes;
		std::vector<VphysHitbox> bsp_hitboxes;
		std::vector<BoundingHitbox> bounding_boxes;
	};

	// Construct a list of all hitboxes around the given position
	Trace::HitboxList Trace::FetchHitboxesAroundPosition(Vector center);
	// Draw all hitboxes in the list
    void DrawHitboxes(Trace::HitboxList list);
	// Draw given vphys hitbox
    void DrawVphysHitbox(Trace::VphysHitbox hitbox);
    // Draw given bounding hitbox
    void DrawBoundingHitbox(Trace::BoundingHitbox hitbox);
}