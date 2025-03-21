#pragma once
#include "Command.hpp"
#include "Interface.hpp"
#include "Module.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

struct IMatRenderContext; // resolve dependency loop

class MaterialSystem : public Module {
public:
	Interface *materials = nullptr;

	uint32_t *renderContextSize = nullptr;
	uint32_t origRenderContextSize = 0;

    using _RenderContextShutdown = void(__cdecl *)(void);
	_RenderContextShutdown RenderContextShutdown = nullptr;

    using _RenderContextInit = void(__cdecl *)(void);
	_RenderContextInit RenderContextInit = nullptr;

	using _RemoveMaterial = void(__rescall*)(void* thisptr, IMaterialInternal* pMaterial);
	_RemoveMaterial RemoveMaterial = nullptr;

	using _KeyValues_SetString = void(__rescall*)(void* thisptr, const char *key, const char *val);
	_KeyValues_SetString KeyValues_SetString = nullptr;

public:
	DECL_DETOUR(UncacheUnusedMaterials, bool bRecomputeStateSnapshots);
	DECL_DETOUR_T(IMaterial *, CreateMaterial, const char *pMaterialName, void *pVMTKeyValues);

	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE("materialsystem"); }

	IMaterial *FindMaterial(const char *materialName, const char *textureGroupName);
	ITexture *CreateTexture(const char *name, int w, int h, uint8_t *bgra);
	void DestroyTexture(ITexture *tex);
	IMatRenderContext *GetRenderContext();
};

extern MaterialSystem *materialSystem;
