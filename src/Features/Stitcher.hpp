#include "Utils/SDK.hpp"

namespace Stitcher {
	void Init(void **videomode);
	void OverrideView(ViewSetup *view);
	void OverrideMovement(CUserCmd *cmd);
	bool Paint();
}
