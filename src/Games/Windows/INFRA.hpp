#pragma once
#include "Portal2.hpp"

class INFRA : public Portal2 {
public:
    INFRA();
    void LoadOffsets() override;
    const char* Version() override;
    const float Tickrate() override;

    static const char* Process();
};
