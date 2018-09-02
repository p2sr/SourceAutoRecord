#pragma once
#include "Module.hpp"

#include "Interface.hpp"
#include "Utils.hpp"

// TODO: Custom fonts
class Scheme : public Module {
public:
    Interface* g_pScheme = nullptr;

    using _GetFont = unsigned long(__func*)(void* thisptr, const char* fontName, bool proportional);
    _GetFont GetFont = nullptr;

public:
    unsigned long GetDefaultFont();

    bool Init() override;
    void Shutdown() override;
    const char* Name() override { return MODULE("vgui2"); }
};

extern Scheme* scheme;
