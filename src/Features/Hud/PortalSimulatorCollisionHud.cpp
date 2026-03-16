#include "Event.hpp"
#include "Variable.hpp"
#include "Modules/Server.hpp"
#include "Modules/Engine.hpp"
#include "Features/OverlayRender.hpp"
#include "Utils/SDK/PortalSimulatorInternalData.hpp"
#include "Features/EntityList.hpp"

Variable sar_portal_simulator_debug("sar_portal_simulator_debug", "0", 0, 1, "Display collision debug meshes for portal simulator physics objects. Requires cheats\n");


static void drawMesh(CPhysCollide *phys_coll, Color c) {
	if (phys_coll == nullptr) {
		return;
	}

	Vector *verts;
	int vert_count = engine->CreateDebugMesh(engine->g_physCollision, phys_coll, &verts);

	MeshId mesh = OverlayRender::createMesh(
		RenderCallback::constant({c.r, c.g, c.b, 20}),
		RenderCallback::constant({c.r, c.g, c.b, 255}));
	for (size_t i = 0; i < vert_count; i += 3) {
		Vector a = verts[i + 0];
		Vector b = verts[i + 1];
		Vector c = verts[i + 2];
		OverlayRender::addTriangle(mesh, a, b, c, true);
	}

	engine->DestroyDebugMesh(engine->g_physCollision, vert_count, verts);
}

static void drawObject(IPhysicsObject *phys, Color c) {
	if (!phys) return;

	using _GetCollide = CPhysCollide *(__rescall *)(const void *thisptr);
	auto GetCollide = Memory::VMT<_GetCollide>(phys, Offsets::GetCollide);
	CPhysCollide *phys_coll = GetCollide(phys);

	drawMesh(phys_coll, c);
}

ON_EVENT(RENDER) {
	if (!sar_portal_simulator_debug.GetBool() || !sv_cheats.GetBool()) {
		return;
	}

	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		void *ent = server->m_EntPtrArray[i].m_pEntity;
		if (!ent) continue;
		auto className = server->GetEntityClassName(ent);

		if (strcmp(className, "prop_portal") != 0) {
			continue;
		}

		auto& portalSimulatorHandle = SE(ent)->field<CPortalSimulator>("m_PortalSimulator");
		auto simulation = portalSimulatorHandle.m_InternalData.Simulation;

		Color staticWorldColor = {0, 255, 150};
		Color staticWallColor = {150, 0, 255};

		if (simulation.Static.World.StaticProps.bCollisionExists) {
			for (int i = 0; i < simulation.Static.World.StaticProps.ClippedRepresentations.m_Size; i++) {
				drawMesh(simulation.Static.World.StaticProps.ClippedRepresentations.m_pElements[i].pCollide, staticWorldColor);
			}
		}

		for (int i = 0; i < 4; i++) {
			drawMesh(simulation.Static.World.Brushes.BrushSets[i].pCollideable, staticWorldColor);
		}

		drawMesh(simulation.Static.World.Displacements.pCollideable, staticWorldColor);

		

		drawMesh(simulation.Static.Wall.Local.Tube.pCollideable, staticWallColor);
		for (int i = 0; i < 4; i++) {
			drawMesh(simulation.Static.Wall.Local.Brushes.BrushSets[i].pCollideable, staticWallColor);
			drawObject(simulation.Static.Wall.RemoteTransformedToLocal.Brushes.pPhysicsObjects[i], staticWallColor);
		}

		for (int i = 0; i < simulation.Static.Wall.RemoteTransformedToLocal.StaticProps.PhysicsObjects.m_Size; i++) {
			auto object = simulation.Static.Wall.RemoteTransformedToLocal.StaticProps.PhysicsObjects.m_pElements[i];
			drawObject(object, staticWallColor);
		}
	}
}