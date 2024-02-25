#include "EntityList.hpp"

#include "Command.hpp"
#include "Modules/Console.hpp"
#include "Modules/Server.hpp"
#include "Modules/Engine.hpp"
#include "Features/Session.hpp"
#include "Features/Camera.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"

#include <algorithm>
#include <cstring>
#include <deque>

EntityList *entityList;

EntityList::EntityList() {
	this->hasLoaded = true;
}
CEntInfo *EntityList::GetEntityInfoByIndex(int index) {
	return reinterpret_cast<CEntInfo *>((uintptr_t)server->m_EntPtrArray + sizeof(CEntInfo) * index);
}
CEntInfo *EntityList::GetEntityInfoByName(const char *name) {
	for (auto index = 0; index < Offsets::NUM_ENT_ENTRIES; ++index) {
		auto info = this->GetEntityInfoByIndex(index);
		if (info->m_pEntity == nullptr) {
			continue;
		}

		auto match = server->GetEntityName(info->m_pEntity);
		if (!match || std::strcmp(match, name) != 0) {
			continue;
		}

		return info;
	}

	return nullptr;
}
CEntInfo *EntityList::GetEntityInfoByClassName(const char *name) {
	for (auto index = 0; index < Offsets::NUM_ENT_ENTRIES; ++index) {
		auto info = this->GetEntityInfoByIndex(index);
		if (info->m_pEntity == nullptr) {
			continue;
		}

		auto match = server->GetEntityClassName(info->m_pEntity);
		if (!match || std::strcmp(match, name) != 0) {
			continue;
		}

		return info;
	}

	return nullptr;
}
int EntityList::GetEntityInfoIndexByHandle(void *entity) {
	if (entity == nullptr) return -1;
	for (auto index = 0; index < Offsets::NUM_ENT_ENTRIES; ++index) {
		auto info = this->GetEntityInfoByIndex(index);
		if (info->m_pEntity != entity) {
			continue;
		}
		return index;
	}
	return -1;
}

bool EntityList::IsPortalGun(const CBaseHandle &handle) {
	if (engine->demoplayer->IsPlaying()) return true;
	if (engine->IsCoop() && engine->IsOrange()) return true; // FIXME: can't get server stuffs
	auto info = this->GetEntityInfoByIndex(handle.GetEntryIndex());
	if (info->m_pEntity == nullptr) {
		return false;
	}

	auto classname = server->GetEntityClassName(info->m_pEntity);
	if (!classname) {
		return false;
	}

	return !std::strcmp(classname, "weapon_portalgun");
}

IHandleEntity *EntityList::LookupEntity(const CBaseHandle &handle) {
	if ((unsigned)handle.m_Index == (unsigned)Offsets::INVALID_EHANDLE_INDEX)
		return NULL;

	auto pInfo = this->GetEntityInfoByIndex(handle.GetEntryIndex());

	if (pInfo->m_SerialNumber == handle.GetSerialNumber())
		return (IHandleEntity *)pInfo->m_pEntity;
	else
		return NULL;
}

// returns an entity with given targetname/classname
// supports array brackets
CEntInfo *EntityList::QuerySelector(const char *selector) {
	int slen = strlen(selector);
	int entId = 0;

	// try to verify if it's a slot number
	int scanEnd;
	if (sscanf(selector, "%d%n", &entId, &scanEnd) == 1 && scanEnd == slen) {
		return entityList->GetEntityInfoByIndex(entId);
	}

	// if not, parse as name instead

	// read list access operator
	if (selector[slen - 1] == ']') {
		int openBracket = slen - 2;
		while (openBracket > 0 && selector[openBracket] != '[') {
			openBracket--;
		}
		if (selector[openBracket] == '[') {
			if (slen - openBracket - 2 > 0) {
				std::string entIdStr(selector + openBracket + 1, slen - openBracket - 2);
				entId = std::atoi(entIdStr.c_str());
			}
			slen = openBracket;
		}
	}
	std::string selectorStr(selector, slen);

	// TODO: maybe implement an * wildcard support here as well?

	for (auto i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {

		auto info = entityList->GetEntityInfoByIndex(i);
		if (info->m_pEntity == nullptr) {
			continue;
		}

		auto tname = server->GetEntityName(info->m_pEntity);
		if (!tname || std::strcmp(tname, selectorStr.c_str()) != 0) {
			auto cname = server->GetEntityClassName(info->m_pEntity);
			if (!cname || std::strcmp(cname, selectorStr.c_str()) != 0) {
				continue;
			}
		}

		if (entId == 0) {
			return info;
		} else {
			entId--;
		}
	}

	return NULL;
}


// Commands

CON_COMMAND(sar_list_ents, "sar_list_ents - lists entities\n") {
	console->Print("[index] (SERIAL) address | m_iClassName | m_iName\n");

	auto pages = Offsets::NUM_ENT_ENTRIES / 512;

	auto page = (args.ArgC() == 2) ? std::atoi(args[1]) : 1;
	page = std::max(page, 1);
	page = std::min(page, pages);

	auto first = (page - 1) * 512;
	auto last = page * 512;

	for (auto index = first; index < Offsets::NUM_ENT_ENTRIES; ++index) {
		if (index == last) {
			break;
		}

		auto info = entityList->GetEntityInfoByIndex(index);
		if (info->m_pEntity == nullptr) {
			continue;
		}

		console->Print("[%i] ", index);
		console->Msg("(%i) ", info->m_SerialNumber);
		console->Msg("%p", info->m_pEntity);
		console->Print(" | ");
		console->Msg("%s", server->GetEntityClassName(info->m_pEntity));
		console->Print(" | ");
		console->Msg("%s\n", server->GetEntityName(info->m_pEntity));
	}
	console->Print("[page %i of %i]\n", page, pages);
}

static void dumpEntInfo(void *entity) {
	if (entity == nullptr) return;
	auto index = entityList->GetEntityInfoIndexByHandle(entity);
	if (index == -1) return;
	auto info = entityList->GetEntityInfoByIndex(index);

	const char *targetname = server->GetEntityName(entity);
	const char *classname = server->GetEntityClassName(entity);

	if (!targetname) targetname = "<no name>";
	if (!classname) classname = "<no class>";

	console->Print("[%i] ", index);
	console->Msg("(%i) ", info->m_SerialNumber);
	console->Msg("%p", info->m_pEntity);
	console->Print(" -> ");
	console->Msg("%s (%s)\n", targetname, classname);

	ICollideable *coll = &SE(entity)->collision();

	ServerEnt *se = (ServerEnt *)entity;
	Vector origin = se->abs_origin();
	console->Print("origin: ");
	console->Msg("%.2f %.2f %.2f\n", origin.x, origin.y, origin.z);

	QAngle angles = se->abs_angles();
	console->Print("angles: ");
	console->Msg("%.2f %.2f %.2f\n", angles.x, angles.y, angles.z);

	Vector velocity = se->abs_velocity();
	console->Print("velocity: ");
	console->Msg("%.2f %.2f %.2f\n", velocity.x, velocity.y, velocity.z);

	console->Print("flags:  ");
	console->Msg("%08X\n", coll->GetSolidFlags());

	if (coll->GetSolidFlags() & FSOLID_NOT_SOLID) {
		console->Msg(" - FSOLID_NOT_SOLID\n");
	}

	if (coll->GetSolidFlags() & FSOLID_NOT_STANDABLE) {
		console->Msg(" - FSOLID_NOT_STANDABLE\n");
	}

	if (coll->GetSolidFlags() & FSOLID_VOLUME_CONTENTS) {
		console->Msg(" - FSOLID_VOLUME_CONTENTS\n");
	}

	console->Print("solid type:  ");
	console->Msg("%d\n", (int)coll->GetSolid());

	console->Print("collision group:  ");
	console->Msg("0x%08X\n", coll->GetCollisionGroup());
}

CON_COMMAND(sar_ent_info, "sar_ent_info [selector] - show info about the entity under the crosshair or with the given name\n") {
	if (!session->isRunning) return;

	if (args.ArgC() == 2) {
		auto entity = entityList->QuerySelector(args[1]);
		if (entity != NULL) {
			dumpEntInfo(entity->m_pEntity);
			return;
		}
	} else {
		CGameTrace tr;

		if (engine->TraceFromCamera(8192, MASK_ALL, tr)) {
			void *entity = tr.m_pEnt;
			if (entity) {
				dumpEntInfo(entity);
				return;
			}
		}
	}
	console->Print("No entity found!\n");
}

CON_COMMAND(sar_find_ents, "sar_find_ents <selector> - finds entities in the entity list by class name\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_find_ents.ThisPtr()->m_pszHelpString);
	}

	console->Print("Results for %s\n", args[1]);
	for (auto index = 0; index < Offsets::NUM_ENT_ENTRIES; ++index) {
		auto info = entityList->GetEntityInfoByIndex(index);
		if (info->m_pEntity == nullptr) {
			continue;
		}

		auto tname = server->GetEntityName(info->m_pEntity);
		if (!tname || std::strcmp(tname, args[1]) != 0) {
			auto cname = server->GetEntityClassName(info->m_pEntity);
			if (!cname || std::strcmp(cname, args[1]) != 0) {
				continue;
			}
		}

		console->Print("[%i] ", index);
		console->Msg("(%i) ", info->m_SerialNumber);
		console->Msg("%p", info->m_pEntity);
		console->Print(" -> ");
		console->Msg("%s\n", server->GetEntityName(info->m_pEntity));
	}
}

std::deque<EntitySlotSerial> g_ent_slot_serial;
CON_COMMAND(sar_ent_slot_serial, "sar_ent_slot_serial <id> [value] - prints entity slot serial number, or sets it if additional parameter is specified.\nBanned in most categories, check with the rules before use!\n") {
	if (client->GetChallengeStatus() == CMStatus::CHALLENGE && !sv_cheats.GetBool()) return console->Print("This is cheating! If you really want to do it, set sv_cheats 1\n");

	if (args.ArgC() < 2 || args.ArgC() > 3) return console->Print(sar_ent_slot_serial.ThisPtr()->m_pszHelpString);

	int id = std::atoi(args[1]);
	if (id < 0 || id > Offsets::NUM_ENT_ENTRIES) {
		return console->Print("Invalid entity slot %d!\n", id);
	}
	if (args.ArgC() == 2) {
		int serial = entityList->GetEntityInfoByIndex(id)->m_SerialNumber;
		console->Print("entity slot %d -> serial number %d\n", id, serial);
	} else {
		int value = std::atoi(args[2]) & 0x7FFF;
		EntitySlotSerial item = EntitySlotSerial();
		item.slot = id;
		item.serial = value;
		if (engine->GetCurrentMapName().length() == 0) {
			entityList->GetEntityInfoByIndex(id)->m_SerialNumber = value;
			item.done = true;
			g_ent_slot_serial.push_back(item);
			console->Print("Serial number of slot %d has been set to %d.\n", id, value);
		} else {
			g_ent_slot_serial.push_back(item);
			console->Print("Serial number of slot %d will be set to %d the next time that slot is removed.\n", id, value);
		}

	}	
}
