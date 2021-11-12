#pragma once

namespace Renderer {
	void Frame();
	void Init(void **videomode);
	void Cleanup();
	extern int segmentEndTick;
	extern bool isDemoLoading;
};  // namespace Renderer
