#include "KeyValues.hpp"

#include "Modules/MaterialSystem.hpp"
#include "Modules/Tier1.hpp"

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

void KeyValues::SetString(const char *key, const char *val) {
	if (materialSystem->KeyValues_SetString) {
		materialSystem->KeyValues_SetString(this, key, val);
	}
}

const char *KeyValues::GetName() {
	auto kvs = tier1->KeyValuesSystem();
	return kvs->GetStringForSymbol((((uint16_t)key_name_case_sensitive_2) << 8) | (uint8_t)(key_name_case_sensitive_1));
}

void *KeyValues::operator new(std::size_t iAllocSize)
{
	auto kvs = tier1->KeyValuesSystem();
	return kvs->AllocKeyValuesMemory(iAllocSize);
}

void KeyValues::operator delete(void *pMem)
{
	auto kvs = tier1->KeyValuesSystem();
	return kvs->FreeKeyValuesMemory(pMem);
}
