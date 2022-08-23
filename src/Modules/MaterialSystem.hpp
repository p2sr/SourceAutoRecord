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

public:
	DECL_DETOUR(UncacheUnusedMaterials, bool bRecomputeStateSnapshots);
	DECL_DETOUR(CreateMaterial, const char *pMaterialName, void *pVMTKeyValues);

	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE("materialsystem"); }

	IMaterial *FindMaterial(const char *materialName, const char *textureGroupName);
	ITexture *CreateTexture(const char *name, int w, int h, uint8_t *bgra);
	IMatRenderContext *GetRenderContext();
};

extern MaterialSystem *materialSystem;
