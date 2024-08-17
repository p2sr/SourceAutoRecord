#include "TraceHitbox.hpp"
#include <Variable.hpp>
#include <Modules/Server.cpp>

using namespace Trace;

Variable sar_trace_bbox_ent_record("sar_trace_bbox_ent_record", "1", "Record hitboxes of nearby entities in the trace. You may want to disable this if memory consumption gets too high.\n");
Variable sar_trace_bbox_ent_draw("sar_trace_bbox_ent_draw", "1", "Draw hitboxes of nearby entities in the trace.\n");
Variable sar_trace_bbox_ent_dist("sar_trace_bbox_ent_dist", "200", 50, "Distance from which to capture entity hitboxes.\n");

bool TryGetSolidColliderFromEntity(void *entity, ICollideable *&collideable) {
	if (!entity || server->IsPlayer(entity)) return false;

	collideable = &SE(entity)->collision();
	if (collideable->GetSolidFlags() & FSOLID_NOT_SOLID) return false;

	return true;
}

bool IsCollidableInDetectionRange(ICollideable *collideable, Vector center) {
	const float d = sar_trace_bbox_ent_dist.GetFloat();

	Vector inclusiveRangeMin = center - Vector{d, d, d};
	Vector inclusiveRangeMax = center + Vector{d, d, d};

	Vector mins, maxs;
	collideable->WorldSpaceSurroundingBounds(&mins, &maxs);
	if (maxs.x < inclusiveRangeMin.x || mins.x > inclusiveRangeMax.x) return false;
	if (maxs.y < inclusiveRangeMin.y || mins.y > inclusiveRangeMax.y) return false;
	if (maxs.z < inclusiveRangeMin.z || mins.z > inclusiveRangeMax.z) return false;

	return true;
}

HitboxList HitboxList::FetchAllNearPosition(Vector center) {
	HitboxList list;
	if (!sar_trace_bbox_ent_record.GetBool()) return list;

	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		void *entity = server->m_EntPtrArray[i].m_pEntity;
		ICollideable *collideable;

		if (!TryGetSolidColliderFromEntity(entity, collideable)) continue;
		if (!IsCollidableInDetectionRange(collideable, center)) continue;

		auto solid_type = collideable->GetSolid();
		bool has_vphys = collideable->GetVPhysicsObject() != nullptr;

		if (solid_type == SOLID_BBOX) {
			list.bounding_boxes.push_back(BoundingHitbox::FromCollideableWorldBounds(collideable));
		} else if (solid_type == SOLID_OBB || solid_type == SOLID_OBB_YAW) {
			list.bounding_boxes.push_back(BoundingHitbox::FromCollideableObjectBounds(collideable));
		} else if (solid_type == SOLID_VPHYSICS && has_vphys) {
			list.vphys_hitboxes.push_back(VphysHitbox::FromCollideable(collideable));
		} else if (solid_type == SOLID_BSP && has_vphys) {
			list.bsp_hitboxes.push_back(VphysHitbox::FromCollideable(collideable));
        }
    }

	return list;
}

BoundingHitbox BoundingHitbox::FromCollideableWorldBounds(ICollideable *collideable) {
	Vector mins, maxs;
	collideable->WorldSpaceSurroundingBounds(&mins, &maxs);
	return BoundingHitbox{mins, maxs, {0, 0, 0}, {0, 0, 0}};
}

BoundingHitbox BoundingHitbox::FromCollideableObjectBounds(ICollideable *collideable) {
	return BoundingHitbox{
		collideable->OBBMins(),
		collideable->OBBMaxs(),
		collideable->GetCollisionOrigin(),
		collideable->GetCollisionAngles(),
	};
}

VphysHitbox VphysHitbox::FromCollideable(ICollideable *collideable) {
	IPhysicsObject *phys = collideable->GetVPhysicsObject();
	if (!phys) return {};

	using _GetCollide = CPhysCollide *(__rescall *)(const void *thisptr);
	auto GetCollide = Memory::VMT<_GetCollide>(phys, Offsets::GetCollide);
	const CPhysCollide *phys_coll = GetCollide(phys);

	Vector *verts;
	int vert_count = engine->CreateDebugMesh(engine->g_physCollision, phys_coll, &verts);

	matrix3x4_t trans = collideable->CollisionToWorldTransform();

	std::vector<Vector> verts_copy(vert_count);
	for (int i = 0; i < vert_count; ++i) {
		verts_copy[i] = trans.VectorTransform(verts[i]);
	}

	engine->DestroyDebugMesh(engine->g_physCollision, vert_count, verts);

	return VphysHitbox{verts_copy};
}


void HitboxList::Draw() const {
	if (!sar_trace_bbox_ent_draw.GetBool()) return;

	for (auto bounding_box : this->bounding_boxes) {
		bounding_box.Draw();
	}
	for (auto vphys_hitbox : this->vphys_hitboxes) {
		vphys_hitbox.Draw();
	}
	for (auto bsp_hitbox : this->bsp_hitboxes) {
		bsp_hitbox.Draw();
	}
}

void VphysHitbox::Draw() const{
	MeshId mesh = OverlayRender::createMesh(
		RenderCallback::constant({255, 0, 0, 20}),
		RenderCallback::constant({255, 0, 0, 255}));
	for (size_t i = 0; i < this->verts.size(); i += 3) {
		Vector a = this->verts[i + 0];
		Vector b = this->verts[i + 1];
		Vector c = this->verts[i + 2];
		OverlayRender::addTriangle(mesh, a, b, c, true);
	}
}

void BoundingHitbox::Draw() const {
	OverlayRender::addBoxMesh(
		this->position, this->mins, this->maxs, this->angles,
		RenderCallback::constant({ 0, 255, 0, 20 }),
		RenderCallback::constant({ 0, 255, 0, 255 })
	);
}
