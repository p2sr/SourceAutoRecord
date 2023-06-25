#pragma once

#include "Math.hpp"
#include "Handle.hpp"
#include "Trace.hpp"

enum SolidType_t {
	SOLID_NONE = 0,
	SOLID_BSP = 1,
	SOLID_BBOX = 2,
	SOLID_OBB = 3,
	SOLID_OBB_YAW = 4,
	SOLID_CUSTOM = 5,
	SOLID_VPHYSICS = 6,
	SOLID_LAST,
};

#define FSOLID_NOT_SOLID 0x04
#define FSOLID_NOT_STANDABLE 0x10
#define FSOLID_VOLUME_CONTENTS 0x20

struct model_t {
	void *fnHandle;
	char szPathName[260];
	int nLoadFlags;
	int nServerCount;
	int type;
	int flags;
	Vector mins, maxs;
	float radius;
};
class IClientUnknown;
class IPhysicsObject;
class CPhysCollide;

class ICollideable {
public:
	virtual IHandleEntity *GetEntityHandle() = 0;
	virtual const Vector &OBBMins() const = 0;
	virtual const Vector &OBBMaxs() const = 0;
	virtual void WorldSpaceTriggerBounds(Vector *worldMins, Vector *worldMaxs) const = 0;
	virtual bool TestCollision(const Ray_t &ray, unsigned int contentsMask, CGameTrace &tr) = 0;
	virtual bool TestHitboxes(const Ray_t &ray, unsigned int contentsMask, CGameTrace &tr) = 0;
	virtual int GetCollisionModelIndex() = 0;
	virtual const model_t *GetCollisionModel() = 0;
	virtual const Vector &GetCollisionOrigin() const = 0;
	virtual const QAngle &GetCollisionAngles() const = 0;
	virtual const matrix3x4_t &CollisionToWorldTransform() const = 0;
	virtual SolidType_t GetSolid() const = 0;
	virtual int GetSolidFlags() const = 0;
	virtual IClientUnknown *GetIClientUnknown() = 0;
	virtual int GetCollisionGroup() const = 0;
	virtual void WorldSpaceSurroundingBounds(Vector *mins, Vector *maxs) = 0;
	virtual unsigned GetRequiredTriggerFlags() const = 0;
	virtual const matrix3x4_t *GetRootParentToWorldTransform() const = 0;
	virtual IPhysicsObject *GetVPhysicsObject() const = 0;
};
