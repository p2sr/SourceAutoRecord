#include "SeamshotFind.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Features/Session.hpp"
#include "Features/Camera.hpp"
#include "Features/OverlayRender.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Variable.hpp"

Variable sar_seamshot_finder("sar_seamshot_finder", "0", 0, 1, "Enables or disables seamshot finder overlay.\n");

SeamshotFind *seamshotFind;

SeamshotFind::SeamshotFind() {
	this->hasLoaded = true;
}

CGameTrace TracePortalShot(const Vector &start, const Vector &dir, float length) {
	CGameTrace tr;

	Vector finalDir = Vector(dir.x, dir.y, dir.z).Normalize() * length;

	Ray_t ray;
	ray.m_IsRay = true;
	ray.m_IsSwept = true;
	ray.m_Start = VectorAligned(start.x, start.y, start.z);
	ray.m_Delta = VectorAligned(finalDir.x, finalDir.y, finalDir.z);
	ray.m_StartOffset = VectorAligned();
	ray.m_Extents = VectorAligned();

	CTraceFilterSimple filter;
	filter.SetPassEntity(server->GetPlayer(GET_SLOT() + 1));

	engine->TraceRay(engine->engineTrace->ThisPtr(), ray, MASK_SHOT_PORTAL, &filter, &tr);

	//hack
	if (ray.m_Start.y == start.y) {
		return tr;
	}
	return tr;
}

ON_EVENT(RENDER) {
	if (sv_cheats.GetBool() && sar_seamshot_finder.GetBool() && !engine->IsSkipping()) {
		void *player = server->GetPlayer(GET_SLOT() + 1);

		if (player == nullptr || (int)player == -1)
			return;

		Vector camPos;
		QAngle camAng;
		camera->GetEyePos<false>(GET_SLOT(), camPos, camAng);

		Vector dir;
		Math::AngleVectors(camAng, &dir);

		CGameTrace tr = TracePortalShot(camPos, dir, 65536.0);

		// did hit something?
		if (tr.plane.normal.Length() > 0.9) {
			// creating 4 vectors for finding the nearest edge
			Vector checkDirs[4];

			Vector upVector = Vector(0, 0, 1);
			if (tr.plane.normal.z * tr.plane.normal.z == 1) {
				upVector = Vector(1, 0, 0);
			}
			//a vector lying on a plane
			checkDirs[0] = tr.plane.normal.Cross(upVector).Normalize();
			//a vector crossing the previous one
			checkDirs[1] = tr.plane.normal.Cross(checkDirs[0]).Normalize();
			//the rest is the inverse of other vectors to get 4 vectors in all directions
			checkDirs[2] = checkDirs[0] * -1;
			checkDirs[3] = checkDirs[1] * -1;

			CGameTrace edgeTr;
			edgeTr.fraction = 999;
			int nearestEdgeID = 0;
			for (int i = 0; i < 4; i++) {
				CGameTrace newEdgeTr = TracePortalShot(tr.endpos + tr.plane.normal, checkDirs[i], 10);
				if (newEdgeTr.fraction < edgeTr.fraction) {
					edgeTr = newEdgeTr;
					nearestEdgeID = i;
				}
			}
			if (edgeTr.fraction < 1) {
				// fixing endpoints
				// for some goddamn reason, end points of ray trace are 0.03125u away from the actual surface.
				// I'm subtracting this offset manually, hoping this will magically fix this bullshit
				// - Krzyhau

				tr.endpos = tr.endpos - tr.plane.normal * 0.03125;
				edgeTr.endpos = edgeTr.endpos - edgeTr.plane.normal * 0.03125;

				// calculate edge point
				float d = (edgeTr.endpos - tr.endpos).Dot(edgeTr.plane.normal) / (checkDirs[nearestEdgeID].Dot(edgeTr.plane.normal));
				Vector edgePoint = tr.endpos + checkDirs[nearestEdgeID] * d;

				//two quick tests for a seamshot.
				Vector side1Vector = tr.plane.normal * 0.1 + edgeTr.plane.normal;
				Vector side2Vector = tr.plane.normal + edgeTr.plane.normal * 0.1;

				//console->Print("edgePoint: (%f, %f, %f)\n", edgePoint.x, edgePoint.y, edgePoint.z);

				Vector test1o = tr.plane.normal * 0.001;
				Vector test2o = edgeTr.plane.normal * 0.001;

				CGameTrace side1tr = TracePortalShot(edgePoint + side1Vector + test1o, side1Vector * -1, 1.5);
				CGameTrace side2tr = TracePortalShot(edgePoint + side2Vector + test2o, side2Vector * -1, 1.5);


				bool seamshotInSide1 = side1tr.plane.normal.Length() == 0;
				bool seamshotInSide2 = side2tr.plane.normal.Length() == 0;
				bool seamshot = seamshotInSide1 || seamshotInSide2;

				int uiScale = 10;

				//calculating vectors for drawing
				Vector edge = edgeTr.plane.normal.Cross(tr.plane.normal).Normalize();

				Vector side1Vec = tr.plane.normal.Cross(edge).Normalize();
				Vector side2Vec = edge.Cross(edgeTr.plane.normal).Normalize();

				MeshId mesh_shot  = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({ uint8_t(seamshot ? 0 : 255), uint8_t(seamshot ? 255 : 0), 0 }, true));
				MeshId mesh_side1 = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({ uint8_t(seamshotInSide1 ? 0 : 255), uint8_t(seamshotInSide1 ? 255 : 0), 0 }, true));
				MeshId mesh_side2 = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({ uint8_t(seamshotInSide2 ? 0 : 255), uint8_t(seamshotInSide2 ? 255 : 0), 0 }, true));

				OverlayRender::addLine(mesh_shot, edgePoint - edge * uiScale, edgePoint + edge * uiScale);

				OverlayRender::addLine(mesh_side1, edgePoint, edgePoint + side1Vec * uiScale);
				OverlayRender::addLine(mesh_side2, edgePoint, edgePoint + side2Vec * uiScale);

				if (seamshot) {
					Vector midPoint = edgePoint + edgeTr.plane.normal * (uiScale / 2.0) + tr.plane.normal * (uiScale / 2.0);
					OverlayRender::addLine(mesh_side1, midPoint, edgePoint + side1Vec * uiScale);
					OverlayRender::addLine(mesh_side2, midPoint, edgePoint + side2Vec * uiScale);
				}

				//engine->AddLineOverlay(nullptr, edgeTr.endpos + test1v + test1o, edgeTr.endpos + test1v * -2 + test1o, 0, 0, 255, true, 0.06);
				//engine->AddLineOverlay(nullptr, edgeTr.endpos + test2v + test2o, edgeTr.endpos + test2v * -2 + test2o, 0, 0, 255, true, 0.06);
			} else {
				int uiScale = 5;
				MeshId mesh = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({ 0, 0, 255 }, true));
				OverlayRender::addLine(mesh, tr.endpos, tr.endpos + tr.plane.normal * uiScale);
				OverlayRender::addLine(mesh, tr.endpos - checkDirs[0] * uiScale, tr.endpos + checkDirs[0] * uiScale);
				OverlayRender::addLine(mesh, tr.endpos - checkDirs[1] * uiScale, tr.endpos + checkDirs[1] * uiScale);
			}
		}
	}
}
