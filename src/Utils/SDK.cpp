#include "SDK.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Modules/Tier1.hpp"
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

KeyValues::KeyValues(const char *name) {
	// bit fields can't have default initialization?!
	this->key_name = 0;
	this->key_name_case_sensitive_1 = 0;

	auto kvs = tier1->KeyValuesSystem();

	int case_insens_key = INVALID_KEY_SYMBOL;
	int case_sens_key = kvs->GetSymbolForStringCaseSensitive(case_insens_key, name);

	this->key_name = case_insens_key;
	this->key_name_case_sensitive_1 = (uint8_t)case_sens_key;
	this->key_name_case_sensitive_2 = (uint16_t)(case_sens_key >> 8);
}

KeyValues::~KeyValues() {
	KeyValues *cur, *next;

	for (cur = this->sub; cur; cur = next) {
		next = cur->peer;
		cur->peer = nullptr;
		delete cur;
	}

	for (cur = this->peer; cur && cur != this; cur = next) {
		next = cur->peer;
		cur->peer = nullptr;
		delete cur;
	}

	delete[] this->val_str;
	delete[] this->val_wstr;
	this->val_str = nullptr;
	this->val_wstr = nullptr;
}

KeyValues *KeyValues::FindKey(const char *name, bool create) {
	auto kvs = tier1->KeyValuesSystem();

	int sym = kvs->GetSymbolForString(name, create);
	if (sym == INVALID_KEY_SYMBOL) return nullptr;

	KeyValues *prev, *cur;
	for (prev = nullptr, cur = this->sub; cur; cur = cur->peer) {
		prev = cur;
		if (cur->key_name == sym) break;
	}

	if (!cur && this->chain) {
		cur = this->chain->FindKey(name, false);
	}

	if (cur) return cur;

	if (!create) return nullptr;

	cur = new KeyValues(name);
	if (prev) {
		prev->peer = cur;
	} else {
		this->sub = cur;
	}
	this->data_type = KeyValues::Type::NONE;

	return cur;
}

void KeyValues::SetInt(const char *key, int val) {
	auto kv = this->FindKey(key, true);
	if (kv) {
		kv->val.i = val;
		kv->data_type = KeyValues::Type::INT;
	}
}
