#pragma once
#include "Command.hpp"
#include "Feature.hpp"
#include "Utils.hpp"

class EntityList : public Feature {
public:
	EntityList();
	CEntInfo *GetEntityInfoByIndex(int index);
	CEntInfo *GetEntityInfoByName(const char *name);
	CEntInfo *GetEntityInfoByClassName(const char *name);
	IHandleEntity *LookupEntity(const CBaseHandle &handle);
};

extern EntityList *entityList;

extern Command sar_list_ents;
extern Command sar_find_ent;
extern Command sar_find_ents;
