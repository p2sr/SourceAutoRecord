#pragma once

namespace Renderer {
    void Frame();
    void Init(void **videomode);
    void Cleanup();
    void OnDemoEnd();
    extern int segmentEndTick;
    extern int demoStart;
    extern bool isDemoLoading;
};
