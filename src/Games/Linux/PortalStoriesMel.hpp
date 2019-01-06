#pragma once
#include "Portal2.hpp"

class PortalStoriesMel : public Portal2 {
public:
    PortalStoriesMel();
    void LoadOffsets() override;
    const char* Version() override;
    const float Tickrate() override;

    static const char* Process();
};
