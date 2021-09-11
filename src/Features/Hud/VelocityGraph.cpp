#include "VelocityGraph.hpp"

#include "Event.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Features/Session.hpp"
#include "Variable.hpp"

#include <deque>

Variable sar_velocitygraph("sar_velocitygraph", "0", "Shows velocity graph.\n");
Variable sar_velocitygraph_font_index("sar_velocitygraph_font_index", "1", 0, "Font index of velocity graph.\n");

VelocityGraph velocityGraph;

struct VelocityData {
	int speed;
	bool on_ground;
};

static std::deque<VelocityData> data[2];

VelocityGraph::VelocityGraph()
	: Hud(HudType_InGame | HudType_Paused | HudType_Menu, true) {
}
bool VelocityGraph::ShouldDraw() {
	return sar_velocitygraph.GetBool() && Hud::ShouldDraw();
}
void VelocityGraph::GatherData(int slot) {
	auto player = client->GetPlayer(slot + 1);

	if (!player) {
		data[slot].clear();
		return;
	}

	if (data[slot].size() > 500)
		data[slot].pop_back();

	VelocityData current_data;

	auto vel = client->GetLocalVelocity(player);
	int speed = vel.Length2D();

	unsigned int groundHandle = *(unsigned int *)((uintptr_t)player + Offsets::C_m_hGroundEntity);
	bool on_ground = groundHandle != 0xFFFFFFFF;

	current_data.speed = speed;
	current_data.on_ground = on_ground;

	data[slot].push_front(current_data);
}

ON_EVENT(PRE_TICK) {
	static int last_tick = 0;

	if (last_tick == session->GetTick()) return;
	last_tick = session->GetTick();

	velocityGraph.GatherData(0);
	velocityGraph.GatherData(1);
}

static int last_vel[2] = {0, 0};
static int tick_prev[2] = {0, 0};

static int take_off[2] = {0, 0};
static int take_off_display_timeout[2] = {0, 0};

void VelocityGraph::Paint(int slot) {
	auto player = client->GetPlayer(slot + 1);

	if (!player) {
		data[slot].clear();

		return;
	}

	auto vel = client->GetLocalVelocity(player);
	int speed = vel.Length2D();

	if (data[slot].size() < 2)
		return;

	int x, y;
#ifdef _WIN32
	engine->GetScreenSize(x, y);
#else
	engine->GetScreenSize(nullptr, x, y);
#endif

	const int graph_pos[2] = {
		x / 2 + 125,
		y - 175
	};

	surface->DrawRect({ 0, 0, 0, 192 }, graph_pos[0] - 250 - 5, graph_pos[1] - 150 - 5, graph_pos[0] + 5, graph_pos[1] + 5);

	for (auto i = 0ul; i < data[slot].size() - 1; i++) {
		const auto current = data[slot][i];
		const auto next = data[slot][i + 1];

		const auto clamped_current_speed = std::clamp(current.speed, 0, 600);
		const auto clamped_next_speed = std::clamp(next.speed, 0, 600);

		int current_speed = (clamped_current_speed * 75 / 320);
		int next_speed = (clamped_next_speed * 75 / 320);

		if (current.on_ground != next.on_ground && !current.on_ground) // forgot to implement this originally
			surface->DrawTxt(scheme->GetDefaultFont(), graph_pos[0] - i / 2, graph_pos[1] - next_speed - 15, Color(255, 255, 255), std::to_string(next.speed).c_str());

		if (i > 0)
			surface->DrawColoredLine(
				graph_pos[0] - (i - 1) / 2,
				graph_pos[1] - current_speed,

				graph_pos[0] - i / 2,
				graph_pos[1] - next_speed,

				Color(255, 255, 255));
	}

	static bool last_on_ground = false;

	unsigned int groundHandle = *(unsigned int *)((uintptr_t)player + Offsets::C_m_hGroundEntity);
	bool on_ground = groundHandle != 0xFFFFFFFF;

	if (last_on_ground && !on_ground) {
		take_off[slot] = speed;
		take_off_display_timeout[slot] = engine->GetClientTime() + 2;
	}

	bool should_draw_takeoff = !on_ground || take_off_display_timeout[slot] > engine->GetClientTime();

	Color c = speed == last_vel[slot] ? Color(255, 199, 89) : speed < last_vel[slot] ? Color(255, 119, 119)	: Color(30, 255, 109);


	auto font = scheme->GetDefaultFont() + sar_velocitygraph_font_index.GetInt();
	auto length = surface->GetFontLength(font, should_draw_takeoff ? "%i (%i)\n" : "%i", speed, take_off[slot]);

	surface->DrawTxt(font, x / 2 - length / 2, y - 150, c, should_draw_takeoff ? "%i (%i)\n" : "%i", speed, take_off[slot]);

	if (tick_prev[slot] + 2 < engine->GetTick()) {
		last_vel[slot] = speed;
		tick_prev[slot] = engine->GetTick();
	}

	last_on_ground = on_ground;
}
bool VelocityGraph::GetCurrentSize(int &xSize, int &ySize) {
	return false;
}
ON_EVENT(SESSION_START) {
	last_vel[0] = 0; // theres probably a lot nicer way to do this
	last_vel[1] = 0;
	tick_prev[0] = 0;
	tick_prev[1] = 0;
	take_off[0] = 0;
	take_off[1] = 0;
	take_off_display_timeout[0] = 0;
	take_off_display_timeout[1] = 0;
}
