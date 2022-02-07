#include "Ruler.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Features/Camera.hpp"
#include "Features/OverlayRender.hpp"
#include "Features/Session.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "SeamshotFind.hpp"
#include "Variable.hpp"
#include "Features/Hud/Hud.hpp"

Variable sar_ruler_grid_align("sar_ruler_grid_align", "1", 0, 1024, "Aligns ruler creation point to the grid of specified size.\n");
Variable sar_ruler_draw("sar_ruler_draw", "1", 0, 4, 
    "Sets the drawing mode of the ruler\n"
    "0 = rulers are not drawn\n"
    "1 = lines, length and angles are drawn (default)\n"
    "2 = only lines and length are drawn\n"
    "3 = only lines are drawn\n"
    "4 = lines, deltas, angles and point origins are drawn\n"
);
Variable sar_ruler_max_trace_dist("sar_ruler_max_trace_dist", "16384", 0, 16384, "Sets maximum trace distance for placing ruler points.\n");
Variable sar_ruler_creator("sar_ruler_creator", "0", 0, 2, 
	"Enables or disables ruler creator\n"
	"0 = Ruler creator disabled\n"
	"1 = Point-to-point ruler creator\n"
	"2 = Player-to-point ruler creator\n"
);


float Ruler::length() {
	return (end - start).Length();
}

QAngle Ruler::angles() {
	Vector forward = (end - start).Normalize();
	float pitch = -atan2(forward.z, forward.Length2D());
	float yaw = atan2(forward.y, forward.x);

	pitch *= 180.0f / M_PI;
	yaw *= 180.0f / M_PI;

	return {pitch, yaw, 0};
}




RulerManager rulerManager;

RulerManager::RulerManager()
	: creationStage(0)
{

}


ON_EVENT(PRE_TICK) {
	rulerManager.UpdateCreator();
}

ON_EVENT(RENDER) {
	if (!sv_cheats.GetBool()) return;
	rulerManager.DrawRulers();
}


void RulerManager::UpdateCreator() {
	if (!IsCreating()) {
		creationStage = 0;
		return;
	}

	//updating trace point

	bool cam_control = sar_cam_control.GetInt() == 1 && sv_cheats.GetBool();

	Vector cam_pos = camera->GetPosition(GET_SLOT());
	Vector dir = camera->GetForwardVector(GET_SLOT()) * sar_ruler_max_trace_dist.GetFloat();

	CGameTrace tr;

	Ray_t ray;
	ray.m_IsRay = true;
	ray.m_IsSwept = true;
	ray.m_Start = VectorAligned(cam_pos.x, cam_pos.y, cam_pos.z);
	ray.m_Delta = VectorAligned(dir.x, dir.y, dir.z);
	ray.m_StartOffset = VectorAligned();
	ray.m_Extents = VectorAligned();

	CTraceFilterSimple filter;
	filter.SetPassEntity(server->GetPlayer(GET_SLOT() + 1));

	engine->TraceRay(engine->engineTrace->ThisPtr(), ray, MASK_SHOT_PORTAL, &filter, &tr);

	// valid trace
	if (tr.plane.normal.Length() > 0.9) {
		creatorTracePoint = tr.endpos;
	} else {
		creatorTracePoint = cam_pos + dir;
	}

	// rounding trace point to the nearest grid point
	float grid = fmaxf(sar_ruler_grid_align.GetFloat(), 0.03125);
	creatorTracePoint = {
		roundf(creatorTracePoint.x / grid) * grid,
		roundf(creatorTracePoint.y / grid) * grid,
		roundf(creatorTracePoint.z / grid) * grid
	};

	// update second point if we're in right creation stage
	if (creationStage == 1) {
		creatorRuler.end = creatorTracePoint;
	}
}


// drawing stuff

// drawing text at defined coordinates
void drawLabel(Vector pos, int yOffset, std::string text) {
	auto font = scheme->GetFontByID(1);
	int font_height = surface->GetFontHeight(font);
	OverlayRender::addText(pos, 0, -font_height - yOffset * font_height - 2, text, font);
}

//drawing point (small box) at defined coordinates
void drawPoint(Vector pos, bool label, Color c) {
	c._color[3] = 1;
	OverlayRender::addBox(pos, {-1, -1, -1}, {1, 1, 1}, {0, 0, 0}, c);
	if (label) {
		auto text = Utils::ssprintf("%.3f %.3f %.3f", pos.x, pos.y, pos.z);
		drawLabel(pos, 0, text);
	}
}

// drawing ruler depending on sar_ruler_draw cvar
void Ruler::draw() {
	int drawMode = sar_ruler_draw.GetInt();
	if (drawMode==0) return;
	// drawing line
	OverlayRender::addLine(start, end, { 0, 100, 200, 255 }, true);

	// drawing points
	bool pointLabels = (drawMode == 4);
	drawPoint(start, pointLabels, {20, 255, 20});
	drawPoint(end, pointLabels, {255, 50, 0});

	// drawing label
	if (drawMode != 3) {
		Vector drawPoint = (end + start) * 0.5;
		int offset = 0;
		if (drawMode != 2) {
			auto angs = angles();
			std::string angLabel = Utils::ssprintf("ang: %.3f %.3f", angs.x, angs.y);
			drawLabel(drawPoint, offset++, angLabel);
		}

		if (drawMode == 4) {
			auto d = (end - start);
			std::string d2 = Utils::ssprintf("dXY: %.3f, dXYZ: %.3f", d.Length2D(), length());
			drawLabel(drawPoint, offset++, d2);
			std::string d1 = Utils::ssprintf("d: %.3f %.3f %.3f ", fabsf(d.x), fabsf(d.y), fabsf(d.z));
			drawLabel(drawPoint, offset++, d1);
		} else {
			std::string lenLabel = Utils::ssprintf("len: %.3f", length());
			drawLabel(drawPoint, offset++, lenLabel);
		}
		
	}
}

void RulerManager::DrawRulers() {

	// draw all of the rulers
	if (rulers.size() > 0) {
		for (Ruler &ruler : rulers) {
			ruler.draw();
		}
	}
	

	// draw creation hud
	if (IsCreating()) {
		if (creationStage >= 1) {
			creatorRuler.draw();
		}
		//drawing trace point
		drawPoint(creatorTracePoint, true, {0, 100, 200});
	}
}





void RulerManager::AddRuler(Vector start, Vector end) {
	rulers.push_back({start, end});
}

void RulerManager::RemoveRuler(int id) {
	if (id < 0 || id >= rulers.size()) return;
	rulers.erase(rulers.begin() + id);
}

void RulerManager::RemoveAllRulers() {
	rulers.clear();
}

// marks current trace point as the next end point for the ruler
// if both end points are defined, putting new ruler into the list
void RulerManager::ProgressCreationStage() {
	if (!IsCreating()) return;
	creationStage++;
	bool saveRuler = false;

	if (sar_ruler_creator.GetInt() == 2) {
		creatorRuler.start = camera->GetPosition(GET_SLOT());
		creatorRuler.end = creatorTracePoint;
		saveRuler = true;
	} else {
		if (creationStage == 1) {
			creatorRuler.start = creatorTracePoint;
			creatorRuler.end = creatorTracePoint; // just because drawing sucks
		} else if (creationStage >= 2) {
			creatorRuler.end = creatorTracePoint;
			if (creatorRuler.length() == 0) {
				// in case it's not a proper line, continue creation process
				creationStage = 1; 
			} else {
				// otherwise just save the line
				saveRuler = true;
			}
		}
	}

	if (saveRuler) {
		AddRuler(creatorRuler.start, creatorRuler.end);
		creationStage = 0;
	}
}

bool RulerManager::IsCreating() {
	return sar_ruler_creator.GetBool();
}




CON_COMMAND(sar_ruler_clear, "sar_ruler_clear - clear all created rulers\n") {
	console->Print("Removing all rulers\n");
	rulerManager.RemoveAllRulers();
}

CON_COMMAND(sar_ruler_creator_set, "sar_ruler_creator_set - sets the point, progressing the ruler creation process.\n") {
	if (!rulerManager.IsCreating()) {
		return console->Print("sar_ruler_creator has to be enabled in order to use this command.\n");
	}
	rulerManager.ProgressCreationStage();
}

CON_COMMAND(sar_ruler_add, "sar_ruler_add <x> <y> <z> <x> <y> <z> - adds a ruler to a set of currently drawn rulers.\n") {
	if (args.ArgC() != 7) {
		return console->Print(sar_ruler_add.ThisPtr()->m_pszHelpString);
	}

	float x0 = atof(args[1]);
	float y0 = atof(args[2]);
	float z0 = atof(args[3]);
	float x1 = atof(args[4]);
	float y1 = atof(args[5]);
	float z1 = atof(args[6]);

	rulerManager.AddRuler({x0, y0, z0}, {x1, y1, z1});
}
