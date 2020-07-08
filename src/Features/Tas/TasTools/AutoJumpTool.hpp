#pragma once
#include "../TasTool.hpp"

struct AutoJumpToolParams : public TasToolParams {
    bool enabled = false;
    AutoJumpToolParams() {}
    AutoJumpToolParams(bool enabled)
        : TasToolParams()
        , enabled(enabled)
    {
    }
};

class AutoJumpTool : public TasTool
{
public:
    AutoJumpTool(const char* name)
        : TasTool(name)
    {
    }

    virtual AutoJumpTool* GetTool();
    virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
    virtual void Apply(TasFramebulk & bulk);
    virtual void Reset();
};

extern AutoJumpTool autoJumpTool;