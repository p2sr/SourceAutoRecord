#pragma once

#include "Math.hpp"
#include "Trace.hpp"

enum PortalPlacementResult_t {
	// Success
	PORTAL_PLACEMENT_SUCCESS,
	PORTAL_PLACEMENT_USED_HELPER,
	PORTAL_PLACEMENT_BUMPED,

	// Fail
	PORTAL_PLACEMENT_CANT_FIT,
	PORTAL_PLACEMENT_CLEANSER,
	PORTAL_PLACEMENT_OVERLAP_LINKED,
	PORTAL_PLACEMENT_OVERLAP_PARTNER_PORTAL,
	PORTAL_PLACEMENT_INVALID_VOLUME,
	PORTAL_PLACEMENT_INVALID_SURFACE,
	PORTAL_PLACEMENT_PASSTHROUGH_SURFACE,
};

struct TracePortalPlacementInfo_t {
	PortalPlacementResult_t ePlacementResult;  // Success indicator
	Vector finalPos;
	QAngle finalAngle;
	void *placementHelper;  // Placement helper if one was hit
	CGameTrace tr;
};
