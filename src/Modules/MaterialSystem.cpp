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

class MemTexRegen : public ITextureRegenerator {
	uint8_t *rgba;
	int w, h;
public:
	MemTexRegen(uint8_t *rgba, int w, int h) : rgba(rgba), w(w), h(h) { }
	virtual void RegenerateTextureBits(ITexture *tex, IVTFTexture *vtf_tex, Rect_t *rect) override {
		auto ImageData = Memory::VMT<uint8_t *(__rescall *)(IVTFTexture *)>(vtf_tex, Offsets::ImageData);
		memcpy(ImageData(vtf_tex), this->rgba, this->w * this->h * 4);
	}
	virtual void Release() override { }
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

DETOUR(MaterialSystem::CreateMaterial, const char *pMaterialName, void *pVMTKeyValues) {
	// rip krzyhau memory leak ultimate fix
	// gone but not forgotten
	/*
    std::string sMaterialName(pMaterialName);
    std::string sMapName(engine->m_szLevelName);

    // Memory leak ultimate fix! -route credits to krzyhau
    // apparently the game loads PeTI related materials into the memory every time you
    // load the game. This simply prevents that from happening.
    bool isPetiMaterial = sMaterialName.find("props_map_editor") != std::string::npos;
    bool isWhiteMaterial = sMaterialName.find("vgui/white") != std::string::npos;
#ifdef _WIN32
    bool isPetiMap = sMapName.find("puzzlemaker\\") != std::string::npos;
#else
    bool isPetiMap = sMapName.find("puzzlemaker/") != std::string::npos;
#endif

    if ((isPetiMaterial || isWhiteMaterial) && !isPetiMap) {
        return 0;
    }
    */

	return MaterialSystem::CreateMaterial(thisptr, pMaterialName, pVMTKeyValues);
}

bool MaterialSystem::Init() {
	this->materials = Interface::Create(this->Name(), "VMaterialSystem080");
	if (this->materials) {
		this->materials->Hook(MaterialSystem::UncacheUnusedMaterials_Hook, MaterialSystem::UncacheUnusedMaterials, 77);
		this->materials->Hook(MaterialSystem::CreateMaterial_Hook, MaterialSystem::CreateMaterial, 81);
		OverlayRender::initMaterials();
	}

	return this->hasLoaded = this->materials;
}
void MaterialSystem::Shutdown() {
	Interface::Delete(this->materials);
}
IMaterial *MaterialSystem::FindMaterial(const char *materialName, const char *textureGroupName) {
	auto func = (IMaterial *(__rescall *)(void *, const char *, const char *, bool, const char *))this->materials->Current(Offsets::FindMaterial);
	return func(this->materials->ThisPtr(), materialName, textureGroupName, true, nullptr);
}
ITexture *MaterialSystem::CreateTexture(const char *name, int w, int h, uint8_t *rgba) {
	auto CreateProceduralTexture = (ITexture *(__rescall *)(void *, const char *, const char *, int, int, ImageFormat, int))this->materials->Current(Offsets::CreateProceduralTexture);
	ITexture *tex = CreateProceduralTexture(this->materials->ThisPtr(), name, "SAR textures", w, h, IMAGE_FORMAT_RGBA8888, TEXTUREFLAGS_NOMIP);
	if (!tex) return nullptr;

	MemTexRegen mtr(rgba, w, h);
	tex->SetTextureRegenerator(&mtr);
	tex->Download();
	tex->SetTextureRegenerator(nullptr);

	return tex;
}
IMatRenderContext *MaterialSystem::GetRenderContext() {
	auto func = (IMatRenderContext *(__rescall *)(void *))this->materials->Current(Offsets::GetRenderContext);
	return func(this->materials->ThisPtr());
}

MaterialSystem *materialSystem;
