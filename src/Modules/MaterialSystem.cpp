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

DETOUR(MaterialSystem::UncacheUnusedMaterials, bool bRecomputeStateSnapshots) {
	auto start = std::chrono::high_resolution_clock::now();
	bool bRecomputeStateSnapshotFixed = sar_prevent_mat_snapshot_recompute.GetBool() ? false : bRecomputeStateSnapshots;
	auto result = MaterialSystem::UncacheUnusedMaterials(thisptr, bRecomputeStateSnapshotFixed);
	auto stop = std::chrono::high_resolution_clock::now();
	console->DevMsg("UncacheUnusedMaterials - %dms\n", std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());
	return result;
}

bool MaterialSystem::Init() {
	this->materials = Interface::Create(this->Name(), "VMaterialSystem080");
	if (this->materials) {
		this->materials->Hook(MaterialSystem::UncacheUnusedMaterials_Hook, MaterialSystem::UncacheUnusedMaterials, Offsets::UncacheUnusedMaterials);

		this->CreateMaterial = this->materials->Original<_CreateMaterial>(Offsets::CreateMaterial);
		this->RemoveMaterial = this->materials->Original<_RemoveMaterial>(Offsets::RemoveMaterial);

#if _WIN32
		this->KeyValues_SetString = (_KeyValues_SetString)Memory::Scan(this->Name(), "55 8B EC 8B 45 08 6A 01 50 E8 ? ? ? ? 85 C0 74 0B");
#else
		if (sar.game->Is(SourceGame_Portal2)) {
			this->KeyValues_SetString = (_KeyValues_SetString)Memory::Scan(this->Name(), "53 83 EC ? 8B 5C 24 ? 6A ? FF 74 24 ? FF 74 24 ? E8 ? ? ? ? 83 C4 ? 85 C0 74 ? 89 5C 24");
		} else {
			this->KeyValues_SetString = (_KeyValues_SetString)Memory::Scan(this->Name(), "55 89 E5 53 83 EC ? 8B 45 ? C7 44 24 ? ? ? ? ? 8B 5D ? 89 44 24 ? 8B 45 ? 89 04 24 E8 ? ? ? ? 85 C0 74 ? 89 5D");
		}
#endif

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
