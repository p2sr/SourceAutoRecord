#pragma once

namespace Renderer {
    void Frame();
    void Init(void **videomode);
    extern int segmentEndTick;
    extern int demoStart;
    extern bool isDemoLoading;
};
