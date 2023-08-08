#pragma once

#include <cstddef>
#include <cstdint>

#define INVALID_KEY_SYMBOL (-1)

class IKeyValuesSystem {
public:
	virtual void RegisterSizeofKeyValues(int size) = 0;
	virtual void *AllocKeyValuesMemory(int size) = 0;
	virtual void FreeKeyValuesMemory(void *mem) = 0;
	virtual int GetSymbolForString(const char *name, bool create = true) = 0;
	virtual const char *GetStringForSymbol(int symbol) = 0;
	virtual void AddKeyValuesToMemoryLeakList(void *mem, int name) = 0;
	virtual void RemoveKeyValuesFromMemoryLeakList(void *mem) = 0;
	virtual void SetKeyValuesExpressionSymbol(const char *name, bool val) = 0;
	virtual bool GetKeyValuesExpressionSymbol(const char *name) = 0;
	virtual int GetSymbolForStringCaseSensitive(int &caseInsensitiveSymbol, const char *name, bool create = true) = 0;
};

typedef bool (*GetSymbolProc_t)(const char *key);

class KeyValues {
public:
	KeyValues(const char *name);
	~KeyValues() = delete;
	KeyValues *FindKey(const char *name, bool create = true);
	void SetInt(const char *key, int val);
	void SetString(const char *key, const char *val);
	const char *GetName();
	void *operator new(std::size_t iAllocSize);
	void operator delete(void *pMem);

	enum class Type : char {
		NONE = 0,
		STRING,
		INT,
		FLOAT,
		PTR,
		WSTRING,
		COLOR,
		UINT64,
		COMPILED_INT_BYTE,
		COMPILED_INT_0,
		COMPILED_INT_1,
		NUMTYPES,
	};

	uint32_t key_name : 24;
	uint32_t key_name_case_sensitive_1 : 8;
	char *val_str = nullptr;
	wchar_t *val_wstr = nullptr;
	union {
		int i;
		float f;
		void *p;
		unsigned char col[4];
	} val;
	Type data_type = Type::NONE;
	bool has_escape_sequences = false;
	uint16_t key_name_case_sensitive_2 = 0;
	KeyValues *peer = nullptr;
	KeyValues *sub = nullptr;
	KeyValues *chain = nullptr;

	GetSymbolProc_t expression_get_symbol_proc = nullptr;
};
