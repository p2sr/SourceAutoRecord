#include "VelocityGraph.hpp"

#include "Event.hpp"
#include "Features/Session.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Utils/SDK/FixedQueue.hpp"
#include "Variable.hpp"

#include <cmath>
#include <vector>

Variable sar_velocitygraph("sar_velocitygraph", "0", "Shows velocity graph.\n");
Variable sar_velocitygraph_font_index("sar_velocitygraph_font_index", "21", 0, "Font index of velocity graph.\n"); // 21 looks pretty good
Variable sar_velocitygraph_show_line("sar_velocitygraph_show_line", "1", "Shows line for velocity graph.\n");
Variable sar_velocitygraph_x("sar_velocitygraph_x", "-250", "Velocity graph x axis offset.\n");
Variable sar_velocitygraph_y("sar_velocitygraph_y", "-175", "Velocity graph y axis offset.\n");
Variable sar_velocitygraph_background("sar_velocitygraph_background", "0", "Background of velocity graph.\n"); // imo this should be off by default
Variable sar_velocitygraph_show_speed_on_graph("sar_velocitygraph_show_speed_on_graph", "1", "Show speed between jumps.\n");
Variable sar_velocitygraph_rainbow("sar_velocitygraph_rainbow", "0", "Rainbow mode of velocity graph text.\n");
Variable sar_velocitygraph_text_color("sar_velocitygraph_text_color", "1", "Whether to color the text of the velocity graph.\n");
Variable sar_velocitygraph_text_groundspeed("sar_velocitygraph_text_groundspeed", "1", "Whether to show the ground speed on the velocity graph text.\n");

struct VelocityData {
	int speed;
	bool on_ground;
	VelocityData(int speed, bool on_ground)
		: speed(speed)
		, on_ground(on_ground) {
	}
	VelocityData()
		: speed(0)
		, on_ground(false) {
	}
};

std::vector<FixedQueue<VelocityData>> velocityStamps(2, FixedQueue(500, VelocityData()));

static int take_off[2] = {0, 0};
static int take_off_display_timeout[2] = {0, 0};

static bool last_on_ground[2] = {false, false};

VelocityGraph::VelocityGraph()
	: Hud(HudType_InGame | HudType_Paused | HudType_Menu, true) {
}
bool VelocityGraph::ShouldDraw() {
	return sar_velocitygraph.GetBool() && Hud::ShouldDraw();
}

void ClearData(int slot) {
	take_off[slot] = 0;
	take_off_display_timeout[slot] = 0;
	velocityStamps[slot].setAll(VelocityData());
}

void VelocityGraph::GatherData(int slot, bool on_ground) {
	auto player = client->GetPlayer(slot + 1);
	if (!player) return;

	auto vel = client->GetLocalVelocity(player);
	int speed = vel.Length2D();

	velocityStamps[slot].Push(VelocityData(speed, on_ground));

	if (last_on_ground[slot] && !on_ground) {
		take_off[slot] = speed;
		take_off_display_timeout[slot] = engine->GetClientTime() + 2;
	}
	last_on_ground[slot] = on_ground;
}

void VelocityGraph::Paint(int slot) {
	int speed = velocityStamps[slot].Last().speed;

	int x, y;
	engine->GetScreenSize(nullptr, x, y);

	Vector2<int> graphPos = Vector2<int>(x / 2, 0) + 
		Vector2<int>(sar_velocitygraph_x.GetInt(), sar_velocitygraph_y.GetInt());
	if (graphPos.y < 0) graphPos.y += y;

	bool should_draw_takeoff = (!last_on_ground[slot] || take_off_display_timeout[slot] > engine->GetClientTime()) && sar_velocitygraph_text_groundspeed.GetBool();
	int recentSpeed = velocityStamps[slot][velocityStamps[slot].size - 10].speed;
	Color c = Color(30, 255, 109);
	if (sar_velocitygraph_text_color.GetBool()) {
		if (sar_velocitygraph_rainbow.GetBool()) {
			c = Utils::HSVToRGB(speed, 100, 100);
		} else if (std::abs(speed - recentSpeed) < 5) {
			c = Color(255, 199, 89);
		} else if (speed < recentSpeed + 5) {
			c = Color(255, 119, 119);
		}
	} else {
		c = Color(255, 255, 255);
	}

	auto font = scheme->GetFontByID(sar_velocitygraph_font_index.GetInt());
	auto length = surface->GetFontLength(font, should_draw_takeoff ? "%i (%i)\n" : "%i", speed, take_off[slot]);

	surface->DrawTxt(font, graphPos.x + 250 - length / 2, graphPos.y + 25, c, should_draw_takeoff ? "%i (%i)\n" : "%i", speed, take_off[slot]);

	if (!sar_velocitygraph_show_line.GetBool()) return;

	for (size_t i = 1; i < velocityStamps[slot].size - 1; i++) {
		const auto current = velocityStamps[slot][i];
		const auto next = velocityStamps[slot][i + 1];

		const auto clamped_current_speed = std::clamp(current.speed, 0, 600);
		const auto clamped_next_speed = std::clamp(next.speed, 0, 600);

		int current_speed = (clamped_current_speed * 75 / 320);
		int next_speed = (clamped_next_speed * 75 / 320);

		if (current.on_ground != next.on_ground && !current.on_ground) {
			if (sar_velocitygraph_show_speed_on_graph.GetBool()) {
				auto height = 15;

				surface->DrawTxt(scheme->GetFontByID(2), graphPos + Vector2<int>(i, -next_speed - height),
					Color(255, 255, 255), std::to_string(next.speed).c_str());
			}
		}
		surface->DrawColoredLine(
			graphPos + Vector2<int>(i - 1, -current_speed),
			graphPos + Vector2<int>(i, -next_speed),
			Color(255, 255, 255));
	}

	if (sar_velocitygraph_background.GetBool())
		surface->DrawRect({0, 0, 0, 192}, graphPos - Vector2<int>(5, 150 + 5), graphPos + Vector2<int>(5 + 500, 5));
}
bool VelocityGraph::GetCurrentSize(int &xSize, int &ySize) {
	return false;
}
ON_EVENT(SESSION_START) {
	for (int slot = 0; slot < 2; slot++) {
		if (take_off[slot] != 0) ClearData(slot);
	}
}
ON_EVENT(PROCESS_MOVEMENT) {
	if (!velocityGraph->ShouldDraw()) {
		if (take_off[event.slot] != 0) ClearData(event.slot);
		return;
	}

	velocityGraph->GatherData(event.slot, event.grounded);
}

VelocityGraph *velocityGraph = new VelocityGraph();
