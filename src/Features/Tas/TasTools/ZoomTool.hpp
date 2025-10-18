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

	inline bool operator==(TasToolParams& rhs_base) {
		auto rhs = *dynamic_cast<ZoomToolParams *>(&rhs_base);
		return TasToolParams::operator==(rhs) 
			&& std::tie(this->zoomType) 
			== std::tie(rhs.zoomType);
	}
};

class ZoomTool : public TasToolWithParams<ZoomToolParams> {
public:
	ZoomTool(int slot)
		: TasToolWithParams("zoom", PRE_PROCESSING, slot) {
	}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
	virtual std::shared_ptr<TasTool> makeCopy();
	virtual void fromCopy(std::shared_ptr<TasTool>);
};

extern ZoomTool zoomTool[2];
