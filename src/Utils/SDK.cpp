#include "SDK.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Platform.hpp"

#include <cstdlib>
#include <cstring>

InterfaceReg *InterfaceReg::s_pInterfaceRegs = nullptr;

static void *CreateInterfaceInternal(const char *pName, int *pReturnCode) {
	InterfaceReg *pCur;

	for (pCur = InterfaceReg::s_pInterfaceRegs; pCur; pCur = pCur->m_pNext) {
		if (!std::strcmp(pCur->m_pName, pName)) {
			if (pReturnCode) {
				*pReturnCode = 0;
			}
			return pCur->m_CreateFn();
		}
	}

	if (pReturnCode) {
		*pReturnCode = 1;
	}
	return nullptr;
}

DLL_EXPORT void *CreateInterface(const char *pName, int *pReturnCode) {
	return CreateInterfaceInternal(pName, pReturnCode);
}

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

const char *variant_t::ToString() const {
	switch (this->fieldType) {
	case FIELD_STRING:
		return this->iszVal;
	case FIELD_INTEGER:
		static char istr[32];
		sprintf(istr, "%i", this->iVal);
		return istr;
	case FIELD_BOOLEAN:
		return this->bVal ? "true" : "false";
	default:
		return "";
	}
}
