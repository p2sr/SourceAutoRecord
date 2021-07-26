#pragma once
#include <vector>

#define DECL_M(name, type) type name(void *entity)
#define MDECL(name, type, offset)                                        \
	type name(void *entity) {                                               \
		return *reinterpret_cast<type *>((uintptr_t)entity + Offsets::offset); \
	}

class Module {
public:
	bool hasLoaded = false;

public:
	virtual ~Module() = default;
	virtual bool Init() = 0;
	virtual void Shutdown() = 0;
	virtual const char *Name() = 0;
};

class Modules {
public:
	std::vector<Module *> list;

public:
	Modules();
	template <typename T = Module>
	void AddModule(T **modulePtr) {
		*modulePtr = new T();
		this->list.push_back(*modulePtr);
	}
	template <typename T = Module>
	void RemoveModule(T **modulePtr) {
		this->list.erase(*modulePtr);
	}
	void InitAll();
	void ShutdownAll();
	void DeleteAll();
	~Modules();
};
