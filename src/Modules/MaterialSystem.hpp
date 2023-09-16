#pragma once
#include "Command.hpp"
#include "Interface.hpp"
#include "Module.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

class IMatRenderContext; // resolve dependency loop

class MaterialSystem : public Module {
public:
	Interface *materials = nullptr;

	using _CreateMaterial = IMaterial*(__rescall*)(void* thisptr, const char *pMaterialName, void *pVMTKeyValues);
	_CreateMaterial CreateMaterial = nullptr;
	
	using _RemoveMaterial = void(__rescall*)(void* thisptr, IMaterialInternal* pMaterial);
	_RemoveMaterial RemoveMaterial = nullptr;

	using _KeyValues_SetString = void(__rescall*)(void* thistpr, const char *key, const char *val);
	_KeyValues_SetString KeyValues_SetString = nullptr;

public:
	DECL_DETOUR(UncacheUnusedMaterials, bool bRecomputeStateSnapshots);

	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE("materialsystem"); }

	IMaterial *FindMaterial(const char *materialName, const char *textureGroupName);
	ITexture *CreateTexture(const char *name, int w, int h, uint8_t *bgra);
	void DestroyTexture(ITexture *tex);
	IMatRenderContext *GetRenderContext();
};

extern MaterialSystem *materialSystem;
