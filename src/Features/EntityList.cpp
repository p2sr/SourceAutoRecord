#include "EntityList.hpp"

#include "Command.hpp"
#include "Modules/Console.hpp"
#include "Modules/Server.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"

#include <algorithm>
#include <cstring>

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
IHandleEntity *EntityList::LookupEntity(const CBaseHandle &handle) {
	if (handle.m_Index == Offsets::INVALID_EHANDLE_INDEX)
		return NULL;

	auto pInfo = this->GetEntityInfoByIndex(handle.GetEntryIndex());

	if (pInfo->m_SerialNumber == handle.GetSerialNumber())
		return (IHandleEntity *)pInfo->m_pEntity;
	else
		return NULL;
}

// Commands

CON_COMMAND(sar_list_ents, "sar_list_ents - lists entities\n") {
	console->Print("[index] address | m_iClassName | m_iName\n");

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
		console->Msg("%p", info->m_pEntity);
		console->Print(" | ");
		console->Msg("%s", server->GetEntityClassName(info->m_pEntity));
		console->Print(" | ");
		console->Msg("%s\n", server->GetEntityName(info->m_pEntity));
	}
	console->Print("[page %i of %i]\n", page, pages);
}
CON_COMMAND(sar_find_ent, "sar_find_ent <m_iName> - finds entity in the entity list by name\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_find_ent.ThisPtr()->m_pszHelpString);
	}

	console->Msg("Results for %s\n", args[1]);
	for (auto index = 0; index < Offsets::NUM_ENT_ENTRIES; ++index) {
		auto info = entityList->GetEntityInfoByIndex(index);
		if (info->m_pEntity == nullptr) {
			continue;
		}

		auto name = server->GetEntityName(info->m_pEntity);
		if (!name || std::strcmp(name, args[1]) != 0) {
			continue;
		}

		console->Print("[%i] ", index);
		console->Msg("%p", info->m_pEntity);
		console->Print(" -> ");
		console->Msg("%s\n", server->GetEntityClassName(info->m_pEntity));
		break;
	}
}
CON_COMMAND(sar_find_ents, "sar_find_ents <m_iClassName> - finds entities in the entity list by class name\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_find_ents.ThisPtr()->m_pszHelpString);
	}

	console->Print("Results for %s\n", args[1]);
	for (auto index = 0; index < Offsets::NUM_ENT_ENTRIES; ++index) {
		auto info = entityList->GetEntityInfoByIndex(index);
		if (info->m_pEntity == nullptr) {
			continue;
		}

		auto cname = server->GetEntityClassName(info->m_pEntity);
		if (!cname || std::strcmp(cname, args[1]) != 0) {
			continue;
		}

		console->Print("[%i] ", index);
		console->Msg("%p", info->m_pEntity);
		console->Print(" -> ");
		console->Msg("%s\n", server->GetEntityName(info->m_pEntity));
	}
}
