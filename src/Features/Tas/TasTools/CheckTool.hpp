#pragma once
#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"

struct CheckToolParams : public TasToolParams {
	CheckToolParams()
		: TasToolParams()
	{}

	CheckToolParams(std::optional<Vector> pos, std::optional<QAngle> ang, float posepsilon, float angepsilon)
		: TasToolParams(true), pos(pos), ang(ang), posepsilon(posepsilon), angepsilon(angepsilon)
	{}

	std::optional<Vector> pos;
	std::optional<QAngle> ang; // roll ignored
	float posepsilon;
	float angepsilon;
};

class CheckTool : public TasTool {
public:
	CheckTool(int slot)
		: TasTool("check", slot)
	{}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &info);
	virtual void Reset();
};

extern CheckTool tasCheckTool[2];
extern int g_tas_check_replays;
