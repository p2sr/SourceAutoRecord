#pragma once
#include "HalfLife2.hpp"

class HalfLifeSource : public HalfLife2 {
public:
    HalfLifeSource();
    const char* Version() override;

    static const char* ModDir();
};
