#pragma once
#include "Modules/InputSystem.hpp"

#include "Commands.hpp"

using namespace Commands;

namespace Rebinder
{
	int SaveButton;
	int ReloadButton;

	std::string SaveName;
	std::string ReloadName;

	bool IsSaveBinding = false;
	bool IsReloadBinding = false;

	// Syncing index between binds
	// Demo recorder can overwrite this
	int LastIndexNumber;

	void SetSaveBind(int button, const char* name)
	{
		SaveButton = button;
		SaveName = std::string(name);
		IsSaveBinding = true;
	}
	void SetReloadBind(int button, const char* name)
	{
		ReloadButton = button;
		ReloadName = std::string(name);
		IsReloadBinding = true;
	}
	void ResetSaveBind()
	{
		IsSaveBinding = false;
		InputSystem::KeySetBinding(SaveButton, "");
	}
	void ResetReloadBind()
	{
		IsReloadBinding = true;
		InputSystem::KeySetBinding(ReloadButton, "");
	}
	void RebindSave()
	{
		if (!IsSaveBinding) return;

		std::string cmd = (LastIndexNumber > 0)
			? std::string("save \"") + SaveName + std::string("_") + std::to_string(LastIndexNumber) + std::string("\"")
			: std::string("save \"") + SaveName + std::string("\"");

		if (sar_save_flag.GetString()[0] != '\0')
			cmd += std::string(";echo \"") + std::string(sar_save_flag.GetString()) + std::string("\"");

		InputSystem::KeySetBinding(SaveButton, cmd.c_str());
	}
	void RebindReload()
	{
		if (!IsReloadBinding) return;

		std::string cmd = (LastIndexNumber > 0)
			? std::string("save \"") + ReloadName + std::string("_") + std::to_string(LastIndexNumber) + std::string("\"")
			: std::string("save \"") + ReloadName + std::string("\"");

		cmd += std::string(";reload");
		InputSystem::KeySetBinding(ReloadButton, cmd.c_str());
	}
	void UpdateIndex(int newIndex)
	{
		LastIndexNumber = newIndex;
	}
}