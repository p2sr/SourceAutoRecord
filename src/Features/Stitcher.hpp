#include "Utils/SDK.hpp"

namespace Stitcher {
	void Init(void **videomode);
	void OverrideView(CViewSetup *view);
	void OverrideMovement(CUserCmd *cmd);
	bool Paint();
}
