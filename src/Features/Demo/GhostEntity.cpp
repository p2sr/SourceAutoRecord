#include "Features/Demo/GhostEntity.hpp"

#include "Features/Demo/DemoGhostPlayer.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/OverlayRender.hpp"
#include "Features/Session.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Modules/Scheme.hpp"
#include "Utils.hpp"
#include "Event.hpp"

#include <cstdlib>
#include <cfloat>

GhostType GhostEntity::ghost_type = GhostType::BENDY;
std::string GhostEntity::defaultModelName = "models/props/food_can/food_can_open.mdl";
Color GhostEntity::set_color{ 0, 0, 0, 255 };
bool GhostEntity::followNetwork;
int GhostEntity::followId = -1;

Variable ghost_height("ghost_height", "16", -256, "Height of the ghosts. (For prop models, only affects their position).\n");
Variable ghost_opacity("ghost_opacity", "255", 0, 255, "Opacity of the ghosts.\n");
Variable ghost_text_offset("ghost_text_offset", "7", -1024, "Offset of the name over the ghosts.\n");
Variable ghost_show_advancement("ghost_show_advancement", "3", 0, 3, "Show the advancement of the ghosts. 1 = show finished runs on the current map, 2 = show all finished runs, 3 = show all finished runs and map changes\n");
Variable ghost_proximity_fade("ghost_proximity_fade", "100", 0, 2000, "Distance from ghosts at which their models fade out.\n");
Variable ghost_shading("ghost_shading", "1", "Enable simple light level based shading for overlaid ghosts.\n");
Variable ghost_show_names("ghost_show_names", "1", "Whether to show names above ghosts.\n");
Variable ghost_name_font_size("ghost_name_font_size", "5.0", 0.1, "The size to render ghost names at.\n");
Variable ghost_spec_thirdperson("ghost_spec_thirdperson", "0", "Whether to spectate ghost from a third-person perspective.\n");
Variable ghost_spec_thirdperson_dist("ghost_spec_thirdperson_dist", "300", 50, "The maximum distance from which to spectate in third-person.\n");
Variable ghost_draw_through_walls("ghost_draw_through_walls", "0", 0, 2, "Whether to draw ghosts through walls. 0 = none, 1 = names, 2 = names and ghosts.\n");

GhostEntity::GhostEntity(unsigned int &ID, std::string &name, DataGhost &data, std::string &current_map, bool network)
	: ID(ID)
	, network(network)
	, name(name)
	, data(data)
	, currentMap(current_map)
	, modelName(GhostEntity::defaultModelName)
	, prop_entity(nullptr)
	, spectator(false)
	, isDestroyed(false)
{
	this->lastUpdate = engine->GetHostTime();
}

GhostEntity::~GhostEntity() {
	this->DeleteGhost();
	if (this->IsBeingFollowed()) {
		GhostEntity::StopFollowing();
	}
}

Color GhostEntity::GetColor() {
	return this->color ? *this->color : GhostEntity::set_color;
}

void GhostEntity::Spawn() {
	if (
		GhostEntity::ghost_type == GhostType::CIRCLE ||
		GhostEntity::ghost_type == GhostType::PYRAMID ||
		GhostEntity::ghost_type == GhostType::BENDY
	) {
		this->prop_entity = nullptr;
		return;
	}

	this->prop_entity = server->CreateEntityByName(nullptr, "prop_dynamic_override");
	if (this->prop_entity == nullptr) {
		console->Warning("CreateEntityByName() failed!\n");
		return;
	}

	server->SetKeyValueChar(nullptr, this->prop_entity, "targetname", "_ghost_normal");

	if (GhostEntity::ghost_type == GhostType::MODEL) {
		server->SetKeyValueChar(nullptr, this->prop_entity, "model", this->modelName.c_str());
	} else {  // GhostType::PYRAMID_PGUN
		server->SetKeyValueChar(nullptr, this->prop_entity, "model", "models/props/prop_portalgun.mdl");
		server->SetKeyValueFloat(nullptr, this->prop_entity, "modelscale", 0.5);
	}

	this->lastOpacity = ghost_opacity.GetFloat();

	server->SetKeyValueChar(nullptr, this->prop_entity, "rendermode", "1");
	server->SetKeyValueFloat(nullptr, this->prop_entity, "renderamt", this->lastOpacity);

	server->DispatchSpawn(nullptr, this->prop_entity);
}

void GhostEntity::DeleteGhost() {
	if (this->prop_entity != nullptr) {
		server->KillEntity(this->prop_entity);
		this->prop_entity = nullptr;
	}
}

void GhostEntity::SetData(DataGhost data, bool network) {
	this->oldPos = this->newPos;
	this->newPos = data;

	auto now = engine->GetHostTime();
	float newLoop = now - this->lastUpdate;
	if (network) {
		// Loop time could do strange things due to network latency etc.
		// Try to smooth it using a biased average of the new time with
		// the old one
		if (this->loopTime == 0.0f) {
			this->loopTime = newLoop;
		} else {
			this->loopTime = (2.0f * this->loopTime + 1.0f * newLoop) / 3.0f;
		}
	} else {
		this->loopTime = newLoop;
	}
	this->lastUpdate = now;

	this->velocity = (this->newPos.position - this->oldPos.position) / this->loopTime;
}

void GhostEntity::SetupGhost(unsigned int &ID, std::string &name, DataGhost &data, std::string &current_map) {
	this->ID = ID;
	this->name = name;
	this->data = data;
	this->currentMap = current_map;
}

void GhostEntity::Display() {
	if (this->IsBeingFollowed() && !ghost_spec_thirdperson.GetBool()) return;

	Color col = GetColor();
	float opacity = ghost_opacity.GetFloat();

	col.r = Utils::ConvertFromSrgb(col.r);
	col.g = Utils::ConvertFromSrgb(col.g);
	col.b = Utils::ConvertFromSrgb(col.b);
	col.a = opacity;

	float prox = ghost_proximity_fade.GetFloat();
	Color fade_col{col.r, col.g, col.b, 0};

	RenderCallback solid = RenderCallback::constant(col, ghost_draw_through_walls.GetInt() >= 2);
	solid = RenderCallback::prox_fade(prox / 2.0, prox, fade_col, this->data.position, solid);// TODO: correct proximity
	if (ghost_shading.GetBool()) {
		solid = RenderCallback::shade(this->data.position + Vector{0,0,10}, solid);
	}

	MeshId mesh = OverlayRender::createMesh(solid, RenderCallback::none);

#define TRIANGLE(p1, p2, p3) OverlayRender::addTriangle(mesh, p1, p2, p3, true)
	switch (GhostEntity::ghost_type) {
	case GhostType::CIRCLE: {
		double rad = ghost_height.GetFloat() / 2;
		Vector origin = this->data.position + Vector(0, 0, rad);

		const int tris = 30;

		auto player = client->GetPlayer(GET_SLOT() + 1);
		Vector pos = player ? client->GetAbsOrigin(player) + client->GetViewOffset(player) : Vector{0, 0, 0};

		float dx = origin.x - pos.x;
		float dy = origin.y - pos.y;
		float hdist = sqrt(dx * dx + dy * dy);

		double yaw =
			origin.x == pos.x && origin.y == pos.y ? M_PI / 2 : atan2(origin.y - pos.y, origin.x - pos.x) + M_PI;

		double pitch =
			hdist == 0 && origin.z == pos.z ? M_PI / 2 : atan2(origin.z - pos.z, hdist);

		double syaw = sin(yaw);
		double cyaw = cos(yaw);
		double spitch = sin(pitch);
		double cpitch = cos(pitch);

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

		for (int i = 0; i < tris; ++i) {
			double lang = M_PI * 2 * i / tris;
			double rang = M_PI * 2 * (i + 1) / tris;

			Vector dl(0, cos(lang) * rad, sin(lang) * rad);
			Vector dr(0, cos(rang) * rad, sin(rang) * rad);

			Vector l = origin + rot * dl;
			Vector r = origin + rot * dr;

			TRIANGLE(r, l, origin);
		}

		break;
	}

	case GhostType::PYRAMID:
	case GhostType::PYRAMID_PGUN: {
		Vector top = this->data.position + Vector{0, 0, ghost_height.GetFloat()};

		Vector a = this->data.position + Vector{5, 5, 0};
		Vector b = this->data.position + Vector{5, -5, 0};
		Vector c = this->data.position + Vector{-5, -5, 0};
		Vector d = this->data.position + Vector{-5, 5, 0};

		TRIANGLE(a, b, top);
		TRIANGLE(b, c, top);
		TRIANGLE(c, d, top);
		TRIANGLE(d, a, top);
		TRIANGLE(b, a, c);
		TRIANGLE(c, a, d);

		break;
	}
	case GhostType::BENDY: {
		// idk, some weird shit was happening when I was setting it in a constructor
		// so I'm just leaving that here
		renderer.SetGhost(this);
		renderer.Draw(mesh);
		break;
	}

	default:
		break;
	}
#undef TRIANGLE

	if (this->prop_entity) {
		if (GhostEntity::ghost_type == GhostType::MODEL) {
			server->SetKeyValueVector(nullptr, this->prop_entity, "origin", this->data.position + Vector{0, 0, ghost_height.GetFloat()});
			server->SetKeyValueVector(nullptr, this->prop_entity, "angles", Vector{this->data.view_angle.x, this->data.view_angle.y, this->data.view_angle.z});
		} else if (GhostEntity::ghost_type == GhostType::PYRAMID_PGUN) {
			float dz = ghost_height.GetFloat() * 2 / 3;
			server->SetKeyValueVector(nullptr, this->prop_entity, "origin", this->data.position + Vector{4, 2, dz});
		}

		if (opacity != this->lastOpacity) {
			server->SetKeyValueFloat(nullptr, this->prop_entity, "renderamt", opacity);
		}

		this->lastOpacity = opacity;
	}

	if (ghost_show_names.GetBool()) this->DrawName();
}

void GhostEntity::Lerp() {
	float time = (engine->GetHostTime() - this->lastUpdate) / this->loopTime;

	if (time > 1) time = 1;
	if (time < 0) time = 0;

	// Try to detect teleportations; if we've moved a massive distance,
	// just teleport to the destination
	bool should_tp = (this->oldPos.position - this->newPos.position).SquaredLength() > 300 * 300;

	if (should_tp) {
		this->data = time < 0.5 ? this->oldPos : this->newPos;
	} else {
		this->data.position.x = (1 - time) * this->oldPos.position.x + time * this->newPos.position.x;
		this->data.position.y = (1 - time) * this->oldPos.position.y + time * this->newPos.position.y;
		this->data.position.z = (1 - time) * this->oldPos.position.z + time * this->newPos.position.z;

		this->data.view_angle.x = (1 - time) * this->oldPos.view_angle.x + time * this->newPos.view_angle.x;
		this->data.view_angle.y = (1 - time) * this->oldPos.view_angle.y + time * this->newPos.view_angle.y;
		this->data.view_angle.z = (1 - time) * this->oldPos.view_angle.z + time * this->newPos.view_angle.z;

		this->data.view_offset = (1 - time) * this->oldPos.view_offset + time * this->newPos.view_offset;
		this->data.grounded = time < 0.5 ? this->oldPos.grounded : this->newPos.grounded;
	}

	// HACK: when using network ghosts, 0,0,0 is used as a sort of
	// "unknown position" identifier, implying that the player hasn't sent
	// any good location data. If we're meant to be there, then just don't
	// display.
	bool valid = false;
	valid |= !!this->data.position.x;
	valid |= !!this->data.position.y;
	valid |= !!this->data.position.z;
	valid |= !!this->data.view_angle.x;
	valid |= !!this->data.view_angle.y;
	valid |= !!this->data.view_angle.z;
	if (valid && (!this->spectator || networkManager.spectator)) this->Display();
}

CON_COMMAND_COMPLETION(ghost_prop_model, "ghost_prop_model <filepath> - set the prop model. Example: models/props/metal_box.mdl\n", ({"models/props/metal_box.mdl", "models/player/chell/player.mdl", "models/player/eggbot/eggbot.mdl", "models/player/ballbot/ballbot.mdl", "models/props/radio_reference.mdl", "models/props/food_can/food_can_open.mdl", "models/npcs/turret/turret.mdl", "models/npcs/bird/bird.mdl"})) {
	if (args.ArgC() <= 1) {
		return console->Print(ghost_prop_model.ThisPtr()->m_pszHelpString);
	}

	GhostEntity::defaultModelName = args[1];

	networkManager.UpdateModel(args[1]);

	demoGhostPlayer.UpdateGhostsModel(args[1]);
}

CON_COMMAND(ghost_type,
            "ghost_type <0/1/2/3/4>:\n"
            "0: Colored circle\n"
            "1: Colored pyramid\n"
            "2: Colored pyramid with portal gun (RECORDED IN DEMOS)\n"
            "3: Prop model (RECORDED IN DEMOS)\n"
            "4: Bendy\n") {
	if (args.ArgC() != 2) {
		return console->Print(ghost_type.ThisPtr()->m_pszHelpString);
	}

	int type = std::atoi(args[1]);

	if (type < 0 || type > 4) {
		return console->Print(ghost_type.ThisPtr()->m_pszHelpString);
	}

	if (GhostEntity::ghost_type != (GhostType)type) {
		GhostEntity::ghost_type = (GhostType)type;
		switch (GhostEntity::ghost_type) {
		case GhostType::CIRCLE:
		case GhostType::PYRAMID:
		case GhostType::BENDY:
			demoGhostPlayer.DeleteAllGhostModels();
			if (networkManager.isConnected) {
				networkManager.DeleteAllGhosts();
			}
			break;
		case GhostType::PYRAMID_PGUN:
		case GhostType::MODEL:
			demoGhostPlayer.DeleteAllGhostModels();
			demoGhostPlayer.SpawnAllGhosts();
			if (networkManager.isConnected) {
				networkManager.DeleteAllGhosts();
				networkManager.SpawnAllGhosts();
			}
			console->Print("WARNING: This ghost_type will invalidate your run! Use type 0 or 1 if you plan to submit the run to leaderboards.\n");
			break;
		}
	}
}

CON_COMMAND(ghost_set_color, "ghost_set_color <hex code> - sets the ghost color to the specified sRGB color code\n") {
	if (args.ArgC() != 2) {
		return console->Print(ghost_set_color.ThisPtr()->m_pszHelpString);
	}

	const char *color = args[1];
	if (color[0] == '#') {
		++color;
	}

	unsigned r, g, b;
	int end;
	if (sscanf(color, "%2x%2x%2x%n", &r, &g, &b, &end) != 3 || end != 6) {
		return console->Print("Invalid color code!\n");
	}

	GhostEntity::set_color = Color{(uint8_t)r,(uint8_t)g,(uint8_t)b};

	if (networkManager.isConnected) {
		networkManager.UpdateColor();
	}
}

void GhostEntity::KillAllGhosts() {
	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		auto info = server->m_EntPtrArray[i];
		if (!info.m_pEntity) {
			continue;
		}

		auto name = server->GetEntityName(info.m_pEntity);
		if (name && !strcmp(name, "_ghost_normal")) {
			server->KillEntity(info.m_pEntity);
		}
	}
}

void GhostEntity::DrawName() {
	Vector nameCoords = this->data.position;
	if (GhostEntity::ghost_type == GhostType::BENDY) {
		nameCoords.z += ghost_text_offset.GetFloat() + renderer.GetHeight();
	} else {
		nameCoords.z += ghost_text_offset.GetFloat() + ghost_height.GetFloat();
	}

	OverlayRender::addText(nameCoords, this->name, ghost_name_font_size.GetFloat(), false, ghost_draw_through_walls.GetInt() >= 1);
}

ON_EVENT(PRE_TICK) {
	if (!sv_cheats.GetBool()) GhostEntity::StopFollowing();
}

GhostEntity *GhostEntity::GetFollowTarget() {
	if (GhostEntity::followId == -1) return nullptr;

	GhostEntity *ghost = GhostEntity::followNetwork ? networkManager.GetGhostByID(GhostEntity::followId).get() : demoGhostPlayer.GetGhostByID(GhostEntity::followId);
	if (!ghost || !ghost->sameMap) return nullptr;

	return ghost;
}

void GhostEntity::FollowPov(CViewSetup *view) {
	GhostEntity *ghost = GhostEntity::GetFollowTarget();
	if (!ghost) return;

	auto pos = ghost->data.position + Vector{0, 0, ghost->data.view_offset};
	auto angles = ghost->data.view_angle;

	if (!ghost_spec_thirdperson.GetBool()) {
		view->origin = pos;
		view->angles = angles;
	} else {
		// Cast a ray out from the ghost in the opposite of the direction we
		// want to view from; that way, we can avoid the camera clipping
		// into a wall

		void *player = server->GetPlayer(1);
		if (!player) return; // Probably shouldn't ever happen

		angles = engine->GetAngles(0);

		CTraceFilterSimple filter;
		filter.SetPassEntity(player);

		Vector forward;
		Math::AngleVectors(angles, &forward);

		const float cam_wall_dist = 32.0f;
		const float max_dist = ghost_spec_thirdperson_dist.GetFloat() + cam_wall_dist;

		Vector delta = forward * -max_dist;

		Ray_t ray;
		ray.m_IsRay = true;
		ray.m_IsSwept = true;
		ray.m_Start = VectorAligned(pos.x, pos.y, pos.z);
		ray.m_Delta = VectorAligned(delta.x, delta.y, delta.z);
		ray.m_StartOffset = {};
		ray.m_Extents = {};

		CGameTrace tr;

		engine->TraceRay(engine->engineTrace->ThisPtr(), ray, MASK_SOLID_BRUSHONLY, &filter, &tr);

		Vector campos = tr.endpos + forward * cam_wall_dist;

		view->origin = campos;
		view->angles = angles;
	}
}

static int r_portalsopenall_value;
static int r_drawviewmodel_value;
static int crosshair_value;

void GhostEntity::StopFollowing() {
	if (GhostEntity::followId == -1) return;
	GhostEntity::followId = -1;
	r_portalsopenall.SetValue(r_portalsopenall_value);
	r_drawviewmodel.SetValue(r_drawviewmodel_value);
	crosshairVariable.SetValue(crosshair_value);
	void *player = server->GetPlayer(1);
	if (player) {
		SE(player)->field<int>("m_fFlags") &= ~FL_GODMODE;
		SE(player)->field<int>("m_fFlags") &= ~FL_NOTARGET;
		SE(player)->field<float>("m_flGravity") = 1.0f;
	}
}

void GhostEntity::StartFollowing(GhostEntity *ghost) {
	if (GhostEntity::followId == -1) {
		r_portalsopenall_value = r_portalsopenall.GetInt();
		r_drawviewmodel_value = r_drawviewmodel.GetInt();
		crosshair_value = crosshairVariable.GetInt();
		r_portalsopenall.SetValue(1);
		r_drawviewmodel.SetValue(0);
		crosshairVariable.SetValue(0);
	}

	GhostEntity::followId = ghost->ID;
	GhostEntity::followNetwork = ghost->network;

	if (!ghost->sameMap) {
		engine->ExecuteCommand("disconnect");
		auto cmd = Utils::ssprintf("map %s", ghost->currentMap.c_str());
		engine->ExecuteCommand(cmd.c_str());
	}

	auto ang = ghost->data.view_angle;
	engine->SetAngles(0, ang);
}

bool GhostEntity::IsBeingFollowed() {
	return GhostEntity::followId == (int)this->ID && GhostEntity::followNetwork == this->network;
}

DECL_COMMAND_COMPLETION(ghost_spec_pov) {
	if (std::strlen(match) == std::strlen(cmd)) {
		items.push_back("none");
	} else if (std::strstr("none", match)) {
		items.push_back("none");
	}

	if (networkManager.isConnected && networkManager.spectator) {
		networkManager.ghostPoolLock.lock();
		for (auto ghost : networkManager.ghostPool) {
			if (items.size() == COMMAND_COMPLETION_MAXITEMS) {
				break;
			}

			if (std::strlen(match) != std::strlen(cmd)) {
				if (std::strstr(ghost->name.c_str(), match)) {
					items.push_back(ghost->name);
				}
			} else {
				items.push_back(ghost->name);
			}
		}
		networkManager.ghostPoolLock.unlock();
	} else {
		for (auto &ghost : demoGhostPlayer.GetAllGhosts()) {
			if (items.size() == COMMAND_COMPLETION_MAXITEMS) {
				break;
			}

			if (std::strlen(match) != std::strlen(cmd)) {
				if (std::strstr(ghost.name.c_str(), match)) {
					items.push_back(ghost.name);
				}
			} else {
				items.push_back(ghost.name);
			}
		}
	}

	FINISH_COMMAND_COMPLETION();
}

CON_COMMAND_F_COMPLETION(ghost_spec_pov, "ghost_spec_pov <name|none> - spectate the specified ghost\n", 0, AUTOCOMPLETION_FUNCTION(ghost_spec_pov)) {
	if (args.ArgC() != 2) {
		return console->Print(ghost_spec_pov.ThisPtr()->m_pszHelpString);
	}

	if (!demoGhostPlayer.IsPlaying() && !networkManager.isConnected) {
		return console->Print("Not playing or connected to a server!\n");
	}

	if (!sv_cheats.GetBool()) {
		return console->Print("ghost_spec_pov requires sv_cheats 1!\n");
	}

	if (!strcmp(args[1], "") || !strcmp(args[1], "none")) {
		GhostEntity::StopFollowing();
		return;
	}

	bool found = false;

	if (networkManager.isConnected && networkManager.spectator) {
		networkManager.ghostPoolLock.lock();
		for (auto ghost : networkManager.ghostPool) {
			if (ghost->name == args[1]) {
				GhostEntity::StartFollowing(ghost.get());
				found = true;
				break;
			}
		}
		networkManager.ghostPoolLock.unlock();
	} else {
		for (auto &ghost : demoGhostPlayer.GetAllGhosts()) {
			if (ghost.name == args[1]) {
				GhostEntity::StartFollowing(&ghost);
				found = true;
				break;
			}
		}
	}

	if (!found) {
		console->Print("No such ghost!\n");
	}
}

CON_COMMAND(ghost_spec_prev, "ghost_spec_prev - spectate the previous ghost\n") {
	if (!sv_cheats.GetBool()) {
		return console->Print("ghost_spec_prev needs sv_cheats 1\n");
	}

	bool same_map = false;
	if (args.ArgC() == 2 && !strcmp(args[1], "samemap")) same_map = true;

	int cur_spec_id = GhostEntity::followId;
	GhostEntity *candidate = nullptr;
	GhostEntity *last = nullptr;

	if (networkManager.isConnected && networkManager.spectator) {
		networkManager.ghostPoolLock.lock();
		for (auto ghost : networkManager.ghostPool) {
			if (ghost->spectator) continue;
			if (same_map && !ghost->sameMap) continue;
			if ((cur_spec_id == -1 || (int)ghost->ID < cur_spec_id) && (!candidate || ghost->ID > candidate->ID)) {
				candidate = ghost.get();
			}
			if (!last || ghost->ID > last->ID) {
				last = ghost.get();
			}
		}
		if (!candidate) candidate = last;
		if (candidate) GhostEntity::StartFollowing(candidate);
		networkManager.ghostPoolLock.unlock();
	} else {
		for (auto &ghost : demoGhostPlayer.GetAllGhosts()) {
			if (same_map && !ghost.sameMap) continue;
			if ((cur_spec_id == -1 || (int)ghost.ID < cur_spec_id) && (!candidate || ghost.ID > candidate->ID)) {
				candidate = &ghost;
			}
			if (!last || ghost.ID > last->ID) {
				last = &ghost;
			}
		}
		if (!candidate) candidate = last;
		if (candidate) GhostEntity::StartFollowing(candidate);
	}

	if (!candidate) {
		console->Print("No ghosts to spectate!\n");
	}
}

CON_COMMAND(ghost_spec_next, "ghost_spec_next [samemap] - spectate the next ghost\n") {
	if (!sv_cheats.GetBool()) {
		return console->Print("ghost_spec_next needs sv_cheats 1\n");
	}

	bool same_map = false;
	if (args.ArgC() == 2 && !strcmp(args[1], "samemap")) same_map = true;

	int cur_spec_id = GhostEntity::followId;
	GhostEntity *candidate = nullptr;
	GhostEntity *first = nullptr;

	if (networkManager.isConnected && networkManager.spectator) {
		networkManager.ghostPoolLock.lock();
		for (auto ghost : networkManager.ghostPool) {
			if (ghost->spectator) continue;
			if (same_map && !ghost->sameMap) continue;
			if ((int)ghost->ID > cur_spec_id && (!candidate || ghost->ID < candidate->ID)) {
				candidate = ghost.get();
			}
			if (!first || ghost->ID < first->ID) {
				first = ghost.get();
			}
		}
		if (!candidate) candidate = first;
		if (candidate) GhostEntity::StartFollowing(candidate);
		networkManager.ghostPoolLock.unlock();
	} else {
		for (auto &ghost : demoGhostPlayer.GetAllGhosts()) {
			if (same_map && !ghost.sameMap) continue;
			if ((int)ghost.ID > cur_spec_id && (!candidate || ghost.ID < candidate->ID)) {
				candidate = &ghost;
			}
			if (!first || ghost.ID < first->ID) {
				first = &ghost;
			}
		}
		if (!candidate) candidate = first;
		if (candidate) GhostEntity::StartFollowing(candidate);
	}

	if (!candidate) {
		console->Print("No ghosts to spectate!\n");
	}
}

HUD_ELEMENT_MODE2(ghost_spec, "0", 0, 1, "Show the name of the ghost you're currently spectating.\n", HudType_InGame | HudType_Paused | HudType_LoadingScreen) {
	auto ghost = GhostEntity::GetFollowTarget();
	ctx->DrawElement("ghost: %s", ghost ? ghost->name.c_str() : "-");
}

// Makes sure some visibility shit is correct
ON_EVENT(PRE_TICK) {
	if (!session->isRunning) return;

	GhostEntity *ghost = GhostEntity::GetFollowTarget();
	if (!ghost) return;

	void *player = server->GetPlayer(1);
	if (!player) return;

	// We use ent_setpos to prevent 'setpos into world' errors being
	// spewed in console
	auto cmd = Utils::ssprintf("ent_setpos 1 %.6f %.6f %.6f", ghost->data.position.x, ghost->data.position.y, ghost->data.position.z);
	engine->ExecuteCommand(cmd.c_str());

	// Make sure we have godmode so we can't die while spectating someone
	SE(player)->field<int>("m_fFlags") |= FL_GODMODE;
	SE(player)->field<int>("m_fFlags") |= FL_NOTARGET;
	SE(player)->field<float>("m_flGravity") = FLT_MIN;
}
