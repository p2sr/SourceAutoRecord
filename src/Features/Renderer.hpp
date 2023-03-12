#pragma once

namespace Renderer {
	void Frame();
	void Init(void **videomode);
	void Cleanup();
	bool IsRunning();
	extern int segmentEndTick;
	extern bool isDemoLoading;
};  // namespace Renderer
