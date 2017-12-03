#pragma once
#include "Modules/InputSystem.hpp"
#include "Commands.hpp"

using namespace Commands;

namespace Rebinder
{
	int SaveButton;
	int ReloadButton;

	const char* SaveName;
	const char* ReloadName;

	bool IsSaveBinding = false;
	bool IsReloadBinding = false;

	// Sync index between binds
	// Also own indexing when not recording with demos
	int LastIndexNumber;

	void SetSaveBind(int button, const char* name) {
		SaveButton = button;
		SaveName = name;
		IsSaveBinding = true;
	}
	void SetReloadBind(int button, const char* name) {
		ReloadButton = button;
		ReloadName = name;
		IsReloadBinding = true;
	}
	void ResetSaveBind() {
		IsSaveBinding = false;
		InputSystem::KeySetBinding(SaveButton, "");
	}
	void ResetReloadBind() {
		IsReloadBinding = true;
		InputSystem::KeySetBinding(ReloadButton, "");
	}
	void RebindSave(int num = 0) {
		if (!IsSaveBinding) return;
		LastIndexNumber = (num == -1) ? LastIndexNumber++ : num;

		char cmd[1024];
		if (sar_save_flag.GetString()[0] == '\0') {
			if (num > 0) {
				snprintf(cmd, sizeof(cmd), "save \"%s_%i\"", SaveName, LastIndexNumber);
			}
			else {
				snprintf(cmd, sizeof(cmd), "save \"%s\"", SaveName);
			}
		}
		else {
			if (num > 0) {
				snprintf(cmd, sizeof(cmd), "save \"%s_%i\";echo \"%s\"", SaveName, LastIndexNumber, sar_save_flag.GetString());
			}
			else {
				snprintf(cmd, sizeof(cmd), "save \"%s\";echo \"%s\"", SaveName, sar_save_flag.GetString());
			}
		}
		InputSystem::KeySetBinding(SaveButton, cmd);
	}
	void RebindReload(int num = 0) {
		if (!IsReloadBinding) return;
		LastIndexNumber = (num == -1) ? LastIndexNumber++ : num;

		char cmd[1024];
		if (num > 0) {
			snprintf(cmd, sizeof(cmd), "save \"%s_%i\";reload", ReloadName, LastIndexNumber);
		}
		else {
			snprintf(cmd, sizeof(cmd), "save \"%s\";reload", ReloadName);
		}
		InputSystem::KeySetBinding(ReloadButton, cmd);
	}
}