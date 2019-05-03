#pragma once
#include "HalfLife2Unpack.hpp"

class PortalUnpack : public HalfLife2Unpack {
public:
    PortalUnpack();
    void LoadOffsets() override;
    const char* Version() override;

    static const char* ModDir();
};
