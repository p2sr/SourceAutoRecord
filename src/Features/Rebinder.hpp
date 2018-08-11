#pragma once
#include <string>

#include "Modules/InputSystem.hpp"

#include "Cheats.hpp"

class Rebinder : public Feature {
public:
    int SaveButton;
    int ReloadButton;
    std::string SaveName;
    std::string ReloadName;
    bool IsSaveBinding;
    bool IsReloadBinding;
    // Syncing index between binds
    // Demo recorder can overwrite this
    int LastIndexNumber;

public:
    Rebinder();
    void SetSaveBind(int button, const char* name);
    void SetReloadBind(int button, const char* name);
    void ResetSaveBind();
    void ResetReloadBind();
    void RebindSave();
    void RebindReload();
    void UpdateIndex(int newIndex);
};

Rebinder::Rebinder()
    : SaveButton(0)
    , ReloadButton(0)
    , SaveName()
    , ReloadName()
    , IsSaveBinding(false)
    , IsReloadBinding(false)
    , LastIndexNumber(0)
{
}
void Rebinder::SetSaveBind(int button, const char* name)
{
    SaveButton = button;
    SaveName = std::string(name);
    IsSaveBinding = true;
}
void Rebinder::SetReloadBind(int button, const char* name)
{
    ReloadButton = button;
    ReloadName = std::string(name);
    IsReloadBinding = true;
}
void Rebinder::ResetSaveBind()
{
    IsSaveBinding = false;
    inputSystem->KeySetBinding(SaveButton, "");
}
void Rebinder::ResetReloadBind()
{
    IsReloadBinding = true;
    inputSystem->KeySetBinding(ReloadButton, "");
}
void Rebinder::RebindSave()
{
    if (!IsSaveBinding)
        return;

    std::string cmd = (LastIndexNumber > 0)
        ? std::string("save \"") + SaveName + std::string("_") + std::to_string(LastIndexNumber) + std::string("\"")
        : std::string("save \"") + SaveName + std::string("\"");

    if (sar_save_flag.GetString()[0] != '\0')
        cmd += std::string(";echo \"") + std::string(sar_save_flag.GetString()) + std::string("\"");

    inputSystem->KeySetBinding(SaveButton, cmd.c_str());
}
void Rebinder::RebindReload()
{
    if (!IsReloadBinding)
        return;

    std::string cmd = (LastIndexNumber > 0)
        ? std::string("save \"") + ReloadName + std::string("_") + std::to_string(LastIndexNumber) + std::string("\"")
        : std::string("save \"") + ReloadName + std::string("\"");

    cmd += std::string(";reload");
    inputSystem->KeySetBinding(ReloadButton, cmd.c_str());
}
void Rebinder::UpdateIndex(int newIndex)
{
    LastIndexNumber = newIndex;
}

Rebinder* rebinder;
extern Rebinder* rebinder;
