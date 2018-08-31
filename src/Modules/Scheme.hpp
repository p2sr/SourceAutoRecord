#pragma once
#include "Module.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

// TODO: Custom fonts
class Scheme : public Module {
public:
    Interface* g_pScheme;

    using _GetFont = unsigned long(__func*)(void* thisptr, const char* fontName, bool proportional);
    _GetFont GetFont;

public:
    unsigned long GetDefaultFont();

    bool Init() override;
    void Shutdown() override;
};

extern Scheme* scheme;
