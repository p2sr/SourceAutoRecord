#include "Features/Demo/GhostEntity.hpp"

#include "Features/Demo/DemoGhostPlayer.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/OverlayRender.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Utils.hpp"

#include <cstdlib>

GhostType GhostEntity::ghost_type = GhostType::BENDY;
std::string GhostEntity::defaultModelName = "models/props/food_can/food_can_open.mdl";
Color GhostEntity::set_color{ 0, 0, 0, 255 };

Variable ghost_height("ghost_height", "16", -256, "Height of the ghosts. (For prop models, only affects their position).\n");
Variable ghost_opacity("ghost_opacity", "255", 0, 255, "Opacity of the ghosts.\n");
Variable ghost_text_offset("ghost_text_offset", "7", -1024, "Offset of the name over the ghosts.\n");
Variable ghost_show_advancement("ghost_show_advancement", "3", 0, 3, "Show the advancement of the ghosts. 1 = show finished runs on the current map, 2 = show all finished runs, 3 = show all finished runs and map changes\n");
Variable ghost_proximity_fade("ghost_proximity_fade", "100", 0, 2000, "Distance from ghosts at which their models fade out.\n");
Variable ghost_shading("ghost_shading", "1", "Enable simple light level based shading for overlaid ghosts.\n");

GhostEntity::GhostEntity(unsigned int &ID, std::string &name, DataGhost &data, std::string &current_map)
	: ID(ID)
	, name(name)
	, data(data)
	, currentMap(current_map)
	, modelName(GhostEntity::defaultModelName)
	, prop_entity(nullptr)
	, isDestroyed(false) {
}

GhostEntity::~GhostEntity() {
	this->DeleteGhost();
}

float GhostEntity::GetOpacity() {
	float opacity = ghost_opacity.GetFloat();

	if (ghost_proximity_fade.GetInt() != 0) {
		auto player = client->GetPlayer(GET_SLOT() + 1);
		if (player) {
			float start_fade_at = ghost_proximity_fade.GetFloat();
			float end_fade_at = ghost_proximity_fade.GetFloat() / 2;

			Vector d = client->GetAbsOrigin(player) - this->data.position;
			float dist = sqrt(d.x * d.x + d.y * d.y + d.z * d.z);  // We can't use squared distance or the fade becomes nonlinear

			if (dist > start_fade_at) {
				// Current value correct
			} else if (dist < end_fade_at) {
				opacity = 0;
			} else {
				float ratio = (dist - end_fade_at) / (start_fade_at - end_fade_at);
				opacity *= ratio;
			}
		}
	}

	return opacity;
}

Color GhostEntity::GetColor() {
	Color c = this->color ? *this->color : GhostEntity::set_color;
	return {c.r(), c.g(), c.b(), (int)GetOpacity()};
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

	auto now = NOW_STEADY();
	long long newLoopTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->lastUpdate).count();
	if (network) {
		// Loop time could do strange things due to network latency etc.
		// Try to smooth it using a biased average of the new time with
		// the old one
		if (this->loopTime == 0) {
			this->loopTime = newLoopTime;
		} else {
			this->loopTime = (2 * this->loopTime + 1 * newLoopTime) / 3;
		}
	} else {
		this->loopTime = newLoopTime;
	}
	this->lastUpdate = now;

	this->velocity = (this->newPos.position - this->oldPos.position) / this->loopTime * 1000.0f;
}

void GhostEntity::SetupGhost(unsigned int &ID, std::string &name, DataGhost &data, std::string &current_map) {
	this->ID = ID;
	this->name = name;
	this->data = data;
	this->currentMap = current_map;
}

void GhostEntity::Display() {
	Color col = GetColor();
	float opacity = col.a();

	if (ghost_shading.GetBool()) OverlayRender::startShading(this->data.position + Vector{0, 0, 5}); // Use a point slightly above the floor

#define TRIANGLE(p1, p2, p3) OverlayRender::addTriangle(p1, p2, p3, col, true)
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
		renderer.Draw();
		break;
	}

	default:
		break;
	}
#undef TRIANGLE

	if (ghost_shading.GetBool()) OverlayRender::endShading();

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
	}

	this->lastOpacity = opacity;
}

void GhostEntity::Lerp(float time) {
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
	if (valid) this->Display();
}

HUD_ELEMENT2(ghost_show_name, "1", "Display the name of the ghost over it.\n", HudType_InGame) {
	if (networkManager.isConnected)
		networkManager.DrawNames(ctx);

	if (demoGhostPlayer.IsPlaying())
		demoGhostPlayer.DrawNames(ctx);
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
            "ghost_type <0/1/2/3>:\n"
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

	int r, g, b;
	int end;
	if (sscanf(color, "%2x%2x%2x%n", &r, &g, &b, &end) != 3 || end != 6) {
		return console->Print("Invalid color code!\n");
	}

	GhostEntity::set_color = Color{
		Utils::ConvertFromSrgb(r),
		Utils::ConvertFromSrgb(g),
		Utils::ConvertFromSrgb(b),
	};

	if (networkManager.isConnected) {
		networkManager.UpdateColor();
	}
}

void GhostEntity::KillAllGhosts() {
	for (size_t i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
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

void GhostEntity::DrawName(HudContext *ctx, int id) {
	Vector nameCoords = this->data.position;
	if (GhostEntity::ghost_type == GhostType::BENDY) {
		nameCoords.z += ghost_text_offset.GetFloat() + renderer.GetHeight();
	} else {
		nameCoords.z += ghost_text_offset.GetFloat() + ghost_height.GetFloat();
	}
	
	Vector screenPos;
	engine->PointToScreen(nameCoords, screenPos);
	ctx->DrawElementOnScreen(id, screenPos.x, screenPos.y, this->name.c_str());
}
