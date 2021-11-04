#pragma once
#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"

class DecelTool : public TasTool {
public:
	DecelTool()
		: TasTool("decel") {}
	virtual DecelTool *GetTool();
	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo);
	virtual void Reset();
};
