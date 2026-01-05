#pragma once

#include "Utils/SDK/ImageFormat.hpp"

namespace Renderer {
	void Frame();
	void Init(void **videomode);
	void Cleanup();
	bool IsRunning();
	void ReadScreenPixels(int x, int y, int w, int h, void *buf, ImageFormat fmt);
	extern int segmentEndTick;
	extern bool isDemoLoading;
};  // namespace Renderer
