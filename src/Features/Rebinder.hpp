#pragma once
#include <string>

#include "Modules/InputSystem.hpp"

#include "Cheats.hpp"

class Rebinder : public Feature {
public:
    int saveButton;
    int reloadButton;
    std::string saveName;
    std::string reloadName;
    bool isSaveBinding;
    bool isReloadBinding;
    // Syncing index between binds
    // Demo recorder can overwrite this
    int lastIndexNumber;

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
    : saveButton(0)
    , reloadButton(0)
    , saveName()
    , reloadName()
    , isSaveBinding(false)
    , isReloadBinding(false)
    , lastIndexNumber(0)
{
    this->hasLoaded = true;
}
void Rebinder::SetSaveBind(int button, const char* name)
{
    this->saveButton = button;
    this->saveName = std::string(name);
    this->isSaveBinding = true;
}
void Rebinder::SetReloadBind(int button, const char* name)
{
    this->reloadButton = button;
    this->reloadName = std::string(name);
    this->isReloadBinding = true;
}
void Rebinder::ResetSaveBind()
{
    this->isSaveBinding = false;
    inputSystem->KeySetBinding(this->saveButton, "");
}
void Rebinder::ResetReloadBind()
{
    this->isReloadBinding = true;
    inputSystem->KeySetBinding(this->reloadButton, "");
}
void Rebinder::RebindSave()
{
    if (!this->isSaveBinding)
        return;

    std::string cmd = std::string("save \"") + this->saveName;
    if (this->lastIndexNumber > 0) {
        cmd += std::string("_") + std::to_string(lastIndexNumber);
    }

    cmd += std::string("\"");

    if (sar_save_flag.GetString()[0] != '\0')
        cmd += std::string(";echo \"") + std::string(sar_save_flag.GetString()) + std::string("\"");

    inputSystem->KeySetBinding(this->saveButton, cmd.c_str());
}
void Rebinder::RebindReload()
{
    if (!this->isReloadBinding)
        return;

    std::string cmd = std::string("save \"") + this->reloadName;
    if (this->lastIndexNumber > 0) {
        cmd += std::string("_") + std::to_string(this->lastIndexNumber);
    }

    cmd += std::string("\";reload");
    inputSystem->KeySetBinding(this->reloadButton, cmd.c_str());
}
void Rebinder::UpdateIndex(int newIndex)
{
    this->lastIndexNumber = newIndex;
}

Rebinder* rebinder;
extern Rebinder* rebinder;
