#pragma once

#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"

struct TestToolParams : public TasToolParams {
    float force = 0;
    TestToolParams(){};
    TestToolParams(float f) : TasToolParams(), force(f) {}
};


class TestTool : public TasTool{
public:
    TestTool(const char* name) : TasTool(name){};
    virtual TestTool* GetTool();
    virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
    virtual void Apply(TasFramebulk& fb); 
    virtual void Reset();
};

extern TestTool tasTestTool;