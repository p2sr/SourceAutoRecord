#pragma once
#include "../TasTool.hpp"

struct JumpToolsParams : public TasToolParams {
	JumpToolsParams()
		: TasToolParams() {
	}
	JumpToolsParams(bool enabled, bool ducked)
		: TasToolParams(enabled)
		, ducked(ducked) {
	}

	bool ducked = false;
};

class JumpTool : public TasToolWithParams<JumpToolsParams> {
public:
	JumpTool(int slot, bool automatic)
		: TasToolWithParams(automatic ? "autojump" : "jump", POST_PROCESSING, BUTTONS, slot) 
		, automatic(automatic) {
	}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);

	bool WillJump(const TasPlayerInfo &pInfo);
private:
	bool ShouldJump(const TasPlayerInfo &pInfo);
	void SetJumpInput(TasFramebulk &bulk, bool jump);

private:
	bool automatic;
	bool hasJumpedLastTick = false;
};

extern JumpTool autoJumpTool[2];
extern JumpTool jumpTool[2];
