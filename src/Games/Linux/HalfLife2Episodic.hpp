#pragma once
#include "HalfLife2.hpp"

class HalfLife2Episodic : public HalfLife2 {
public:
    HalfLife2Episodic();
    const char* Version() override;

    static const char* ModDir();
};
