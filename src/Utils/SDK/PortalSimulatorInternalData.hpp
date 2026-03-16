#pragma once

#include "Color.hpp"
#include "ICollideable.hpp"
#include "Math.hpp"
#include "UtlMemory.hpp"

// cleaned up version of portalsimulation.h (assuming server-side!!!)

class CPolyhedron;
class IPhysicsEnvironment;

class CPortalSimulatorEventCallbacks  // sends out notifications of events to game specific code
{
public:
	virtual void PortalSimulator_TookOwnershipOfEntity(void *pEntity) {};
	virtual void PortalSimulator_ReleasedOwnershipOfEntity(void *pEntity) {};

	virtual void PortalSimulator_TookPhysicsOwnershipOfEntity(void *pEntity) {};
	virtual void PortalSimulator_ReleasedPhysicsOwnershipOfEntity(void *pEntity) {};
};

struct PropPolyhedronGroup_t {
	int iStartIndex;
	int iNumPolyhedrons;
};

enum PortalSimulationEntityFlags_t {
	PSEF_OWNS_ENTITY = (1 << 0),
	PSEF_OWNS_PHYSICS = (1 << 1),
	PSEF_IS_IN_PORTAL_HOLE = (1 << 2),
	PSEF_CLONES_ENTITY_FROM_MAIN = (1 << 3),
	PSEF_CLONES_ENTITY_ACROSS_PORTAL_FROM_MAIN = (1 << 4),
};

enum PS_PhysicsObjectSourceType_t {
	PSPOST_LOCAL_BRUSHES,
	PSPOST_REMOTE_BRUSHES,
	PSPOST_LOCAL_STATICPROPS,
	PSPOST_REMOTE_STATICPROPS,
	PSPOST_HOLYWALL_TUBE,
	PSPOST_LOCAL_DISPLACEMENT,
};

enum RayInPortalHoleResult_t {
	RIPHR_NOT_TOUCHING_HOLE = 0,
	RIPHR_TOUCHING_HOLE_NOT_WALL,
	RIPHR_TOUCHING_HOLE_AND_WALL,
};

struct PortalTransformAsAngledPosition_t {
	Vector ptOriginTransform;
	QAngle qAngleTransform;
	Vector ptShrinkAlignedOrigin;
};


struct PS_PlacementData_t {
	Vector ptCenter;
	QAngle qAngles;
	Vector vForward;
	Vector vUp;
	Vector vRight;
	float fHalfWidth, fHalfHeight;
	VPlane PortalPlane;
	VMatrix matThisToLinked;
	VMatrix matLinkedToThis;
	PortalTransformAsAngledPosition_t ptaap_ThisToLinked;
	PortalTransformAsAngledPosition_t ptaap_LinkedToThis;
	CPhysCollide *pHoleShapeCollideable;
	CPhysCollide *pInvHoleShapeCollideable;
	CPhysCollide *pAABBAngleTransformCollideable;
	Vector vecCurAABBMins;
	Vector vecCurAABBMaxs;
	Vector vCollisionCloneExtents;
	CBaseHandle hPortalPlacementParent;
	bool bParentIsVPhysicsSolidBrush;
};

struct PS_SD_Static_CarvedBrushCollection_t {
	CUtlVector<CPolyhedron *> Polyhedrons;
	CPhysCollide *pCollideable;
	IPhysicsObject *pPhysicsObject;
};

struct PS_SD_Static_BrushSet_t : public PS_SD_Static_CarvedBrushCollection_t {
	int iSolidMask;
};

struct PS_SD_Static_World_Brushes_t {
	PS_SD_Static_BrushSet_t BrushSets[4];
};

struct PS_SD_Static_World_Displacements_t {
	CPhysCollide *pCollideable;
	IPhysicsObject *pPhysicsObject;
};


struct PS_SD_Static_World_StaticProps_ClippedProp_t {
	PropPolyhedronGroup_t PolyhedronGroup;
	CPhysCollide *pCollide;
	IPhysicsObject *pPhysicsObject;
	IHandleEntity *pSourceProp;

	int iTraceContents;
	short iTraceSurfaceProps;
	static void *pTraceEntity;
	static const char *szTraceSurfaceName;
	static const int iTraceSurfaceFlags;
};

struct PS_SD_Static_World_StaticProps_t {
	CUtlVector<CPolyhedron *> Polyhedrons;
	CUtlVector<PS_SD_Static_World_StaticProps_ClippedProp_t> ClippedRepresentations;
	bool bCollisionExists;
	bool bPhysicsExists;
};

struct PS_SD_Static_World_t {
	PS_SD_Static_World_Brushes_t Brushes;
	PS_SD_Static_World_Displacements_t Displacements;
	PS_SD_Static_World_StaticProps_t StaticProps;
};

struct PS_SD_Static_Wall_Local_Tube_t {
	CUtlVector<CPolyhedron *> Polyhedrons;
	CPhysCollide *pCollideable;
	IPhysicsObject *pPhysicsObject;
};

struct PS_SD_Static_Wall_Local_Brushes_t {
	PS_SD_Static_BrushSet_t BrushSets[4];
	PS_SD_Static_CarvedBrushCollection_t Carved_func_clip_vphysics;
};

struct PS_SD_Static_Wall_Local_t {
	PS_SD_Static_Wall_Local_Tube_t Tube;
	PS_SD_Static_Wall_Local_Brushes_t Brushes;
};

struct PS_SD_Static_Wall_RemoteTransformedToLocal_Brushes_t {
	IPhysicsObject *pPhysicsObjects[4];
};

struct PS_SD_Static_Wall_RemoteTransformedToLocal_StaticProps_t {
	CUtlVector<IPhysicsObject *> PhysicsObjects;
};

struct PS_SD_Static_Wall_RemoteTransformedToLocal_t {
	PS_SD_Static_Wall_RemoteTransformedToLocal_Brushes_t Brushes;
	PS_SD_Static_Wall_RemoteTransformedToLocal_StaticProps_t StaticProps;
};

struct PS_SD_Static_Wall_t {
	PS_SD_Static_Wall_Local_t Local;
	PS_SD_Static_Wall_RemoteTransformedToLocal_t RemoteTransformedToLocal;
};

struct PS_SD_Static_SurfaceProperties_t {
	int contents;
	csurface_t surface;
	void *pEntity;
};

struct PS_SD_Static_t {
	PS_SD_Static_World_t World;
	PS_SD_Static_Wall_t Wall;
	PS_SD_Static_SurfaceProperties_t SurfaceProperties;
};

class CPhysicsShadowClone;

struct PS_SD_Dynamic_PhysicsShadowClones_t {
	CUtlVector<void *> ShouldCloneFromMain;
	CUtlVector<CPhysicsShadowClone *> FromLinkedPortal;

	CUtlVector<void *> ShouldCloneToRemotePortal;
};


struct PS_SD_Dynamic_CarvedEntities_CarvedEntity_t {
	PropPolyhedronGroup_t UncarvedPolyhedronGroup;
	PropPolyhedronGroup_t CarvedPolyhedronGroup;
	CPhysCollide *pCollide;
	IPhysicsObject *pPhysicsObject;
	void *pSourceEntity;
};

struct PS_SD_Dynamic_CarvedEntities_t {
	bool bCollisionExists;
	bool bPhysicsExists;
	CUtlVector<CPolyhedron *> Polyhedrons;
	CUtlVector<PS_SD_Dynamic_CarvedEntities_CarvedEntity_t> CarvedRepresentations;
};

struct PS_SD_Dynamic_t {
	unsigned int EntFlags[2048];
	CUtlVector<void *> OwnedEntities;
	PS_SD_Dynamic_PhysicsShadowClones_t ShadowClones;
	uint32_t HasCarvedVersionOfEntity[(2048 + (sizeof(uint32_t) * 8) - 1) / (sizeof(uint32_t) * 8)];
	PS_SD_Dynamic_CarvedEntities_t CarvedEntities;
};

class CPortalSimulator;
class CPSCollisionEntity;

struct PS_SimulationData_t {
	void *vtable;
	PS_SD_Static_t Static;
	PS_SD_Dynamic_t Dynamic;
	IPhysicsEnvironment *pPhysicsEnvironment;
};

struct PS_DebuggingData_t {
	Color overlayColor;
};

struct PS_InternalData_t {
	void *vtable;
	PS_PlacementData_t Placement;
	PS_SimulationData_t Simulation;
	PS_DebuggingData_t Debugging;
};

struct CPortalSimulator {
public:
	void *vtable;
	bool m_bLocalDataIsReady;
	bool m_bSimulateVPhysics;
	bool m_bGenerateCollision;
	bool m_bSharedCollisionConfiguration;
	CPortalSimulator *m_pLinkedPortal;
	bool m_bInCrossLinkedFunction;
	CPortalSimulatorEventCallbacks *m_pCallbacks;
	int m_iPortalSimulatorGUID;

	struct
	{
		bool bPolyhedronsGenerated;
		bool bLocalCollisionGenerated;
		bool bLinkedCollisionGenerated;
		bool bLocalPhysicsGenerated;
		bool bLinkedPhysicsGenerated;
	} m_CreationChecklist;

	PS_InternalData_t m_InternalData;
};