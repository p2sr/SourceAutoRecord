#include "TracePortal.hpp"
#include <Variable.hpp>
#include <Modules/Server.cpp>

using namespace Trace;

Variable sar_trace_portal_record("sar_trace_portal_record", "1", "Record portal locations.\n");
Variable sar_trace_portal_oval("sar_trace_portal_oval", "0", "Draw trace portals as ovals rather than rectangles.\n");
Variable sar_trace_portal_opacity("sar_trace_portal_opacity", "100", 0, 255, "Opacity of trace portal previews.\n");

bool IsEntityActivePortal(void *entity) {
	if (!entity || server->IsPlayer(entity)) return false;
	if (strcmp(server->GetEntityClassName(entity), "prop_portal")) return false;
	if (!SE(entity)->field<bool>("m_bActivated")) return false;

	return true;
}

Trace::PortalLocation GetPortalLocationFromEntity(void *ent) {
	Trace::PortalLocation portal;

	portal.position = server->GetAbsOrigin(ent);
	portal.angles = server->GetAbsAngles(ent);
	portal.is_primary = !SE(ent)->field<bool>("m_bIsPortal2");
	portal.is_coop = engine->IsCoop();

	if (portal.is_coop) {
		CBaseHandle shooter_handle = SE(ent)->field<CBaseHandle>("m_hFiredByPlayer");
		void *shooter = entityList->LookupEntity(shooter_handle);
		portal.is_atlas = shooter == server->GetPlayer(1);
	}

	return portal;
}

PortalsList PortalsList::FetchCurrentLocations() {
	Trace::PortalsList portals;

	if (!sar_trace_portal_record.GetBool()) return portals;

	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		void *ent = server->m_EntPtrArray[i].m_pEntity;
		
		if (IsEntityActivePortal(ent)) {
			auto portal = GetPortalLocationFromEntity(ent);
			portals.locations.push_back(portal);
		}
	}

	return portals;
}

void PortalsList::Draw() const {
	for (auto portal : this->locations) {
		portal.Draw();
	}
}

void PortalLocation::Draw() const {
    Vector origin = this->GetDrawOrigin();
	Matrix rotation = this->GetDrawRotationMatrix();

	MeshId mesh = OverlayRender::createMesh(RenderCallback::constant(this->GetColor()), RenderCallback::none);

	const int HALF_WIDTH = 32;
	const int HALF_HEIGHT = 56;

	if (sar_trace_portal_oval.GetBool()) {
		const int OVAL_RESOLUTION = 20;
		for (int i = 0; i < OVAL_RESOLUTION; ++i) {
			double lang = M_PI * 2 * i / OVAL_RESOLUTION;
			double rang = M_PI * 2 * (i + 1) / OVAL_RESOLUTION;

			Vector dl(0, HALF_WIDTH * cos(lang), HALF_HEIGHT * sin(lang));
			Vector dr(0, HALF_WIDTH * cos(rang), HALF_HEIGHT * sin(rang));

			Vector l = origin + rotation * dl;
			Vector r = origin + rotation * dr;

			OverlayRender::addTriangle(mesh, l, r, origin);
		}
	} else {
		OverlayRender::addQuad(
			mesh,
			origin + rotation * Vector{0, -HALF_WIDTH, -HALF_HEIGHT},
			origin + rotation * Vector{0, -HALF_WIDTH, HALF_HEIGHT},
			origin + rotation * Vector{0, HALF_WIDTH, HALF_HEIGHT},
			origin + rotation * Vector{0, HALF_WIDTH, -HALF_HEIGHT}
		);
	}

	// small triangle indicator for up direction
	OverlayRender::addTriangle(
		mesh,
		origin + rotation * Vector{0, -5, 56},
		origin + rotation * Vector{0, 0, 64},
		origin + rotation * Vector{0, 5, 56}
	);
}

Color PortalLocation::GetColor() const {
	int portalIndex = is_primary ? 1 : 2;
	int playerIndex = is_coop ? (is_atlas ? 3 : 2) : 0;

	Color portalColor = SARUTIL_Portal_Color(portalIndex, playerIndex);
	portalColor.a = (uint8_t)sar_trace_portal_opacity.GetInt();
}

Vector PortalLocation::GetDrawOrigin() const {
	const int BIGGER_THAN_DIST_EPSILON = 0.04;

	Vector forwardVector;
	Math::AngleVectors(this->angles, &forwardVector);
	return this->position + forwardVector * BIGGER_THAN_DIST_EPSILON;
}

Matrix PortalLocation::GetDrawRotationMatrix() const {
	double syaw = sin(DEG2RAD(this->angles.y));
	double cyaw = cos(DEG2RAD(this->angles.y));
	double spitch = sin(DEG2RAD(this->angles.x));
	double cpitch = cos(DEG2RAD(this->angles.x));

	// yaw+pitch rotation matrix
	Matrix rot{3, 3, 0};
	rot(0, 0) = cyaw * cpitch;
	rot(0, 1) = -syaw;
	rot(0, 2) = cyaw * spitch;
	rot(1, 0) = syaw * cpitch;
	rot(1, 1) = cyaw;
	rot(1, 2) = syaw * spitch;
	rot(2, 0) = -spitch;
	rot(2, 1) = 0;
	rot(2, 2) = cpitch;

	return rot;
}
