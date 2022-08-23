#include "Teleporter.hpp"

#include "Command.hpp"
#include "Entity.hpp"
#include "Event.hpp"
#include "Features/EntityList.hpp"
#include "Features/NetMessage.hpp"
#include "Features/Session.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

#define TELEPORT_MESSAGE_TYPE "teleport"

#define TP_OP_SAVE 0
#define TP_OP_RESTORE 1
#define TP_OP_SETANG 2
#define TP_OP_MSG 3

static void handleMessage(const void *data, size_t size) {
	if (size < 2) return;

	int op = ((char *)data)[0];
	int slot = ((char *)data)[1];

	if (op == TP_OP_SAVE) {
		if (size != 14) return;
		QAngle ang;
		ang.x = *(float *)((uintptr_t)data + 2);
		ang.y = *(float *)((uintptr_t)data + 6);
		ang.z = *(float *)((uintptr_t)data + 10);
		if (!engine->IsOrange()) teleporter->SaveLocal(slot, ang);
	} else if (op == TP_OP_RESTORE) {
		if (size != 3) return;
		bool portals = ((char *)data)[2] != 0;
		if (!engine->IsOrange()) teleporter->TeleportLocal(slot, portals);
	} else if (op == TP_OP_SETANG) {
		if (size != 14) return;
		QAngle ang;
		ang.x = *(float *)((uintptr_t)data + 2);
		ang.y = *(float *)((uintptr_t)data + 6);
		ang.z = *(float *)((uintptr_t)data + 10);
		engine->SetAngles(engine->IsOrange() ? 0 : slot, ang);
	} else if (op == TP_OP_MSG) {
		console->Print("%.*s\n", size - 2, (char *)data + 2);
	}
}

ON_INIT { NetMessage::RegisterHandler(TELEPORT_MESSAGE_TYPE, &handleMessage); }

static void sendMessage(int slot, const void *data, size_t size) {
	int local = engine->IsOrange() ? 1 : 0;
	if (engine->IsSplitscreen() || slot == local) {
		handleMessage(data, size);
	} else {
		NetMessage::SendMsg(TELEPORT_MESSAGE_TYPE, data, size);
	}
}

Teleporter *teleporter;

Teleporter::Teleporter()
	: locations() {
	for (auto i = 0; i < Offsets::MAX_SPLITSCREEN_PLAYERS; ++i) {
		this->locations.push_back(TeleportLocation());
	}

	this->hasLoaded = true;
}
TeleportLocation &Teleporter::GetLocation(int slot) {
	return this->locations[slot];
}
void Teleporter::SaveLocal(int slot, QAngle ang) {
	auto player = server->GetPlayer(slot + 1);
	if (!player) return;

	auto &location = this->GetLocation(slot);
	location.origin = server->GetAbsOrigin(player);
	location.angles = ang;
	location.velocity = server->GetLocalVelocity(player);

	for (int i = 0; i < 2; ++i) {
		location.portals[i].isSet = false;
	}

	auto portalGun = entityList->LookupEntity(player->active_weapon());
	if (portalGun) {
		unsigned char linkage = SE(portalGun)->field<unsigned char>("m_iPortalLinkageGroupID");
		void *bluePortal = (void *)server->FindPortal(linkage, false, false);
		void *orangePortal = (void *)server->FindPortal(linkage, true, false);
		for (int i = 0; i < 2; ++i) {
			auto portal = i == 0 ? bluePortal : orangePortal;
			location.portals[i].linkage = linkage;
			if (portal && SE(portal)->field<bool>("m_bActivated")) {
				location.portals[i].pos = server->GetAbsOrigin(portal);
				location.portals[i].ang = server->GetAbsAngles(portal);
				location.portals[i].isSet = true;
			} else {
				location.portals[i].isSet = false;
			}
		}
	}

	location.isSet = true;

	std::string msg = Utils::ssprintf("Saved location: %.3f %.3f %.3f", location.origin.x, location.origin.y, location.origin.z);
	char *buf = (char *)malloc(msg.size() + 2);
	buf[0] = TP_OP_MSG;
	buf[1] = slot;
	memcpy(buf + 2, msg.c_str(), msg.size());
	sendMessage(slot, buf, msg.size() + 2);
	free(buf);
}
void Teleporter::TeleportLocal(int slot, bool portals) {
	if (!sv_cheats.GetBool()) return;

	auto &location = this->GetLocation(slot);
	if (!location.isSet) {
		const char msg[] = "Location not set! Use sar_teleport_setpos.";
		char buf[sizeof msg + 1]; // 1 not 2 because null terminator
		buf[0] = TP_OP_MSG;
		buf[1] = slot;
		memcpy(buf+2, msg, sizeof msg - 1);
		sendMessage(slot, buf, sizeof buf);
		return;
	}

	uintptr_t player = (uintptr_t)server->GetPlayer(slot + 1);
	if (!player) return;

	SE(player)->field<Vector>("m_vecVelocity") = location.velocity;
	SE(player)->field<int>("m_iEFlags") |= (1<<12); // EFL_DIRTY_ABSVELOCITY

	char setpos[64];
	std::snprintf(setpos, sizeof(setpos), "setpos_player %d %f %f %f", slot + 1, location.origin.x, location.origin.y, location.origin.z);
	engine->ExecuteCommand(setpos);

	if (portals) {
		for (int i = 0; i < 2; ++i) {
			if (location.portals[i].isSet) {
				auto cmd = Utils::ssprintf(
					"portal_place %d %d %.6f %.6f %.6f %.6f %.6f %.6f",
					(int)location.portals[i].linkage,
					i,
					location.portals[i].pos.x,
					location.portals[i].pos.y,
					location.portals[i].pos.z,
					location.portals[i].ang.x,
					location.portals[i].ang.y,
					location.portals[i].ang.z
				);
				engine->ExecuteCommand(cmd.c_str());
			} else {
				auto portal = server->FindPortal(location.portals[i].linkage, i == 1, false);
				if (portal) SE(portal)->field<bool>("m_bActivated") = false;
			}
		}
	}

	char data[14];
	data[0] = TP_OP_SETANG;
	data[1] = slot;
	*(float *)(data + 2) = location.angles.x;
	*(float *)(data + 6) = location.angles.y;
	*(float *)(data + 10) = location.angles.z;
	sendMessage(slot, data, sizeof data);
}
void Teleporter::Save(int slot) {
	auto ang = engine->GetAngles(engine->IsOrange() ? 0 : slot);
	char data[14];
	data[0] = TP_OP_SAVE;
	data[1] = slot;
	*(float *)(data + 2) = ang.x;
	*(float *)(data + 6) = ang.y;
	*(float *)(data + 10) = ang.z;
	sendMessage(0, data, sizeof data);
}
void Teleporter::Teleport(int slot, bool portals) {
	char data[3];
	data[0] = TP_OP_RESTORE;
	data[1] = slot;
	data[2] = portals ? 1 : 0;
	sendMessage(0, data, sizeof data);
}

// Commands

CON_COMMAND(sar_teleport, "sar_teleport [noportals] - teleports the player to the last saved location\n") {
	IGNORE_DEMO_PLAYER();

	bool portals = true;
	if (args.ArgC() > 1 && !strcmp(args[1], "noportals")) portals = false;

	if (!sv_cheats.GetBool()) {
		return console->Print("Cannot teleport without sv_cheats 1!\n");
	}

	teleporter->Teleport(GET_SLOT(), portals);
}
CON_COMMAND(sar_teleport_setpos, "sar_teleport_setpos - saves current location for teleportation\n") {
	teleporter->Save(GET_SLOT());
}
