#include "EntityEdict.hpp"

#include "Modules/Server.hpp"

inline int ENTINDEX(edict_t *pEdict) {
	return (pEdict) ? pEdict - server->gpGlobals->pEdicts : 0;
}
inline edict_t *INDEXENT(int iEdictNum) {
	if (server->gpGlobals->pEdicts) {
		auto pEdict = server->gpGlobals->pEdicts + iEdictNum;
		return (pEdict->IsFree()) ? nullptr : pEdict;
	}
	return nullptr;
}
