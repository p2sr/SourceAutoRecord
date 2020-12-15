#pragma once

#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"

struct TestToolParams : public TasToolParams {
    float force = 0;
    TestToolParams()
        : TasToolParams()
    {
    }
    TestToolParams(float f)
        : TasToolParams(true)
        , force(f)
    {
    }
};


class TestTool : public TasTool{
public:
    TestTool(const char* name) : TasTool(name){};
    virtual TestTool* GetTool();
    virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
    virtual void Apply(TasFramebulk& fb, const TasPlayerInfo& pInfo); 
    virtual void Reset();
};

extern TestTool tasTestTool;