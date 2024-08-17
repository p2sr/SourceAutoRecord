#pragma once

#include <Utils/SDK/Math.hpp>
#include <Utils/SDK/ICollideable.hpp>
#include <vector>

namespace Trace {
	struct VphysHitbox {
		std::vector<Vector> verts;

		void Draw() const;
		static VphysHitbox FromCollideable(ICollideable *collideable);
	};

	struct BoundingHitbox {
		Vector mins, maxs;
		Vector position;
		QAngle angles;

		void Draw() const;
		static BoundingHitbox FromCollideableWorldBounds(ICollideable *collideable);
		static BoundingHitbox FromCollideableObjectBounds(ICollideable *collideable);
	};

	struct HitboxList {
		std::vector<VphysHitbox> vphys_hitboxes;
		std::vector<VphysHitbox> bsp_hitboxes;
		std::vector<BoundingHitbox> bounding_boxes;

		void Draw() const;
		static HitboxList FetchAllNearPosition(Vector center);
	};
}