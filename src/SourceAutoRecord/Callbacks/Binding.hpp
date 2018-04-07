#pragma once
#include "Modules/ConCommandArgs.hpp"
#include "Modules/Console.hpp"
#include "Modules/InputSystem.hpp"

#include "Features/Rebinder.hpp"

namespace Callbacks
{
	void BindSaveRebinder(const void* ptr)
	{
		ConCommandArgs args(ptr);
		if (args.Count() != 3) {
			Console::Print("sar_bind_save <key> [save_name] : Automatic save rebinding when server has loaded. File indexing will be synced when recording demos.\n");
			return;
		}

		int button = InputSystem::GetButton(args.At(1));
		if (button == BUTTON_CODE_INVALID) {
			Console::Print("\"%s\" isn't a valid key!\n", args.At(1));
			return;
		}
		else if (button == KEY_ESCAPE) {
			Console::Print("Can't bind ESCAPE key!\n", args.At(1));
			return;
		}

		if (Rebinder::IsReloadBinding && button == Rebinder::ReloadButton) {
			Rebinder::ResetReloadBind();
		}

		Rebinder::SetSaveBind(button, args.At(2));
		Rebinder::RebindSave();
	}
	void BindReloadRebinder(const void* ptr)
	{
		ConCommandArgs args(ptr);
		if (args.Count() != 3) {
			Console::Print("sar_bind_reload <key> [save_name] : Automatic save-reload rebinding when server has loaded. File indexing will be synced when recording demos.\n");
			return;
		}

		int button = InputSystem::GetButton(args.At(1));
		if (button == BUTTON_CODE_INVALID) {
			Console::Print("\"%s\" isn't a valid key!\n", args.At(1));
			return;
		}
		else if (button == KEY_ESCAPE) {
			Console::Print("Can't bind ESCAPE key!\n", args.At(1));
			return;
		}

		if (Rebinder::IsSaveBinding && button == Rebinder::SaveButton) {
			Rebinder::ResetSaveBind();
		}

		Rebinder::SetReloadBind(button, args.At(2));
		Rebinder::RebindReload();
	}
	void UnbindSaveRebinder()
	{
		if (!Rebinder::IsSaveBinding) {
			Console::Print("There's nothing to unbind.\n");
			return;
		}
		Rebinder::ResetSaveBind();
	}
	void UnbindReloadRebinder()
	{
		if (!Rebinder::IsReloadBinding) {
			Console::Print("There's nothing to unbind.\n");
			return;
		}
		Rebinder::ResetReloadBind();
	}
}