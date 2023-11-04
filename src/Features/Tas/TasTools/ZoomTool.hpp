#pragma once
#include "../TasTool.hpp"

enum ZoomType {
	In,
	Out,
	Toggle
};

struct ZoomToolParams : public TasToolParams {
	ZoomToolParams()
		: TasToolParams() {
	}
	ZoomToolParams(bool enabled, ZoomType zoomType)
		: TasToolParams(enabled)
		, zoomType(zoomType) {
	}

	ZoomType zoomType = In;
};

class ZoomTool : public TasToolWithParams<ZoomToolParams> {
public:
	ZoomTool(int slot)
		: TasToolWithParams("zoom", slot) {
	}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
};

extern ZoomTool zoomTool[2];
