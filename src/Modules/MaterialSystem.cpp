#include "MaterialSystem.hpp"

#include "Cheats.hpp"
#include "Command.hpp"
#include "Engine.hpp"
#include "Interface.hpp"
#include "Module.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"
#include "Features/OverlayRender.hpp"

#define RENDERCONTEXT_ALLOC_SIZE 0x672000 // 6.45 MB

class MemTexRegen : public ITextureRegenerator {
	uint8_t *bgra;
	int w, h;

public:
	MemTexRegen(uint8_t *bgra, int w, int h) : w(w), h(h) {
		this->bgra = new uint8_t[w * h * 4];
		memcpy(this->bgra, bgra, w * h * 4);
	}

	~MemTexRegen() {
		delete[] this->bgra;
	}

	virtual void RegenerateTextureBits(ITexture *tex, IVTFTexture *vtf_tex, Rect_t *rect) override {
		auto ImageFormat_fn = Memory::VMT<ImageFormat (__rescall *)(IVTFTexture *)>(vtf_tex, Offsets::ImageFormat);
		ImageFormat fmt = ImageFormat_fn(vtf_tex);

		if (fmt != IMAGE_FORMAT_BGRA8888) {
			console->Print("ERROR: MemTexRegen got unexpected image format %d\n", (int)fmt);
			return;
		}

		auto ImageData = Memory::VMT<uint8_t *(__rescall *)(IVTFTexture *)>(vtf_tex, Offsets::ImageData);
		memcpy(ImageData(vtf_tex), this->bgra, this->w * this->h * 4);
	}

	virtual void Release() override {
		delete this;
	}
};

REDECL(MaterialSystem::UncacheUnusedMaterials);
REDECL(MaterialSystem::CreateMaterial);

DETOUR(MaterialSystem::UncacheUnusedMaterials, bool bRecomputeStateSnapshots) {
	auto start = std::chrono::high_resolution_clock::now();
	bool bRecomputeStateSnapshotFixed = sar_prevent_mat_snapshot_recompute.GetBool() ? false : bRecomputeStateSnapshots;
	auto result = MaterialSystem::UncacheUnusedMaterials(thisptr, bRecomputeStateSnapshotFixed);
	auto stop = std::chrono::high_resolution_clock::now();
	console->DevMsg("UncacheUnusedMaterials - %dms\n", std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());
	return result;
}

DETOUR_T(IMaterial *, MaterialSystem::CreateMaterial, const char *pMaterialName, void *pVMTKeyValues) {
	if (sar.game->Is(SourceGame_Portal2 | SourceGame_Portal2_2011 | SourceGame_PortalReloaded | SourceGame_PortalStoriesMel)) {
		// Patched by Valve, no need for us to do it again
		return MaterialSystem::CreateMaterial(thisptr, pMaterialName, pVMTKeyValues);
	}

	std::string sMaterialName(pMaterialName);
	std::string sMapName(engine->m_szLevelName);
	std::replace(sMaterialName.begin(), sMaterialName.end(), '\\', '/');
	std::replace(sMapName.begin(), sMapName.end(), '\\', '/');

	// Memory leak ultimate fix! -route credits to krzyhau
    // apparently the game loads PeTI related materials into the memory every time you
    // load the game. This simply prevents that from happening.
	// Revived from the dead for mods!
    bool isPetiMaterial = sMaterialName.find("props_map_editor") != std::string::npos;
    bool isWhiteMaterial = sMaterialName.find("vgui/white") != std::string::npos;
    bool isPetiMap = sMapName.find("puzzlemaker/") != std::string::npos;
    if ((isPetiMaterial || isWhiteMaterial) && !isPetiMap) {
        return 0;
    }

	return MaterialSystem::CreateMaterial(thisptr, pMaterialName, pVMTKeyValues);
}

bool MaterialSystem::Init() {
	this->materials = Interface::Create(this->Name(), "VMaterialSystem080");
	if (this->materials) {
		this->materials->Hook(MaterialSystem::UncacheUnusedMaterials_Hook, MaterialSystem::UncacheUnusedMaterials, Offsets::UncacheUnusedMaterials);
		this->materials->Hook(MaterialSystem::CreateMaterial_Hook, MaterialSystem::CreateMaterial, Offsets::CreateMaterial);

		this->RemoveMaterial = this->materials->Original<_RemoveMaterial>(Offsets::RemoveMaterial);

		this->KeyValues_SetString = (_KeyValues_SetString)Memory::Scan(this->Name(), Offsets::KeyValues_SetString);

		OverlayRender::initMaterials();
	}

	this->renderContextSize = Memory::Scan<uint32_t *>(this->Name(), Offsets::RenderContextSize, Offsets::RenderContextSizeOff);
	this->RenderContextShutdown = Memory::Scan<_RenderContextShutdown>(this->Name(), Offsets::RenderContextShutdown);
	this->RenderContextInit = Memory::Scan<_RenderContextInit>(this->Name(), Offsets::RenderContextInit);
	if (this->renderContextSize && this->RenderContextShutdown && this->RenderContextInit) {
		if (*this->renderContextSize != RENDERCONTEXT_ALLOC_SIZE) {
			Memory::UnProtect((void *)this->renderContextSize, sizeof(uint32_t));
			this->origRenderContextSize = *this->renderContextSize;
			*this->renderContextSize = RENDERCONTEXT_ALLOC_SIZE;
			this->RenderContextShutdown();
			this->RenderContextInit();
		}
	}

	return this->hasLoaded = this->materials;
}
void MaterialSystem::Shutdown() {
	Interface::Delete(this->materials);

	if (origRenderContextSize) {
		*renderContextSize = origRenderContextSize;
		RenderContextShutdown();
		RenderContextInit();
	}
}
IMaterial *MaterialSystem::FindMaterial(const char *materialName, const char *textureGroupName) {
	auto func = (IMaterial *(__rescall *)(void *, const char *, const char *, bool, const char *))this->materials->Current(Offsets::FindMaterial);
	return func(this->materials->ThisPtr(), materialName, textureGroupName, true, nullptr);
}
ITexture *MaterialSystem::CreateTexture(const char *name, int w, int h, uint8_t *bgra) {
	auto CreateProceduralTexture = (ITexture *(__rescall *)(void *, const char *, const char *, int, int, ImageFormat, int))this->materials->Current(Offsets::CreateProceduralTexture);
	// For any 8-bit alpha raw format, the regeneration will use BGRA8888 in the
	// VTF texture, so we may as well mirror that in the actual texture too. Note
	// that this also means the argument to this function must be BGRA unless we
	// implement some custom conversion logic in MemTexRegen
	ITexture *tex = CreateProceduralTexture(this->materials->ThisPtr(), name, "SAR textures", w, h, IMAGE_FORMAT_BGRA8888, TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_SINGLECOPY);
	if (!tex) return nullptr;

	tex->SetTextureRegenerator(new MemTexRegen(bgra, w, h));
	tex->Download();

	return tex;
}
void MaterialSystem::DestroyTexture(ITexture *tex) {
	if (tex) {
		auto CreateProceduralTexture = this->materials->Current(Offsets::CreateProceduralTexture);
		auto g_pTextureManager = Memory::DerefDeref(CreateProceduralTexture + Offsets::g_pTextureManager);
		auto RemoveTexture = Memory::VMT<void(__rescall*)(void*, ITexture*)>(g_pTextureManager, Offsets::RemoveTexture);
		RemoveTexture(g_pTextureManager, tex);
		tex = nullptr;
	}
}
IMatRenderContext *MaterialSystem::GetRenderContext() {
	auto func = (IMatRenderContext *(__rescall *)(void *))this->materials->Current(Offsets::GetRenderContext);
	return func(this->materials->ThisPtr());
}

MaterialSystem *materialSystem;
