#include "MaterialSystem.hpp"

#include "Cheats.hpp"
#include "Command.hpp"
#include "Engine.hpp"
#include "Interface.hpp"
#include "Module.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

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
	}

	return this->hasLoaded = this->materials;
}
void MaterialSystem::Shutdown() {
	Interface::Delete(this->materials);
}

MaterialSystem *materialSystem;
