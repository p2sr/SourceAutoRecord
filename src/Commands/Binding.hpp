#pragma once
#include "Modules/Console.hpp"
#include "Modules/InputSystem.hpp"

#include "Features/Rebinder.hpp"

#include "Command.hpp"

CON_COMMAND(sar_bind_save,
    "Automatic save rebinding when server has loaded.\n"
    "File indexing will be synced when recording demos.\n"
    "Usage: sar_bind_save <key> [save_name]\n")
{
    if (args.ArgC() != 3) {
        console->Print("sar_bind_save <key> [save_name] : Automatic save rebinding when server has loaded. File indexing will be synced when recording demos.\n");
        return;
    }

    int button = inputSystem->GetButton(args[1]);
    if (button == BUTTON_CODE_INVALID) {
        console->Print("\"%s\" isn't a valid key!\n", args[1]);
        return;
    } else if (button == KEY_ESCAPE) {
        console->Print("Can't bind ESCAPE key!\n", args[1]);
        return;
    }

    if (rebinder->IsReloadBinding && button == rebinder->ReloadButton) {
        rebinder->ResetReloadBind();
    }

    rebinder->SetSaveBind(button, args[2]);
    rebinder->RebindSave();
}

CON_COMMAND(sar_bind_reload,
    "Automatic save-reload rebinding when server has loaded.\n"
    "File indexing will be synced when recording demos.\n"
    "Usage: sar_bind_reload <key> [save_name]\n")
{
    if (args.ArgC() != 3) {
        console->Print("sar_bind_reload <key> [save_name] : Automatic save-reload rebinding when server has loaded. File indexing will be synced when recording demos.\n");
        return;
    }

    int button = inputSystem->GetButton(args[1]);
    if (button == BUTTON_CODE_INVALID) {
        console->Print("\"%s\" isn't a valid key!\n", args[1]);
        return;
    } else if (button == KEY_ESCAPE) {
        console->Print("Can't bind ESCAPE key!\n", args[1]);
        return;
    }

    if (rebinder->IsSaveBinding && button == rebinder->SaveButton) {
        rebinder->ResetSaveBind();
    }

    rebinder->SetReloadBind(button, args[2]);
    rebinder->RebindReload();
}

CON_COMMAND(sar_unbind_save,
    "Unbinds current save rebinder.\n")
{
    if (!rebinder->IsSaveBinding) {
        console->Print("There's nothing to unbind.\n");
        return;
    }
    rebinder->ResetSaveBind();
}

CON_COMMAND(sar_unbind_reload,
    "Unbinds current save-reload rebinder.\n")
{
    if (!rebinder->IsReloadBinding) {
        console->Print("There's nothing to unbind.\n");
        return;
    }
    rebinder->ResetReloadBind();
}
