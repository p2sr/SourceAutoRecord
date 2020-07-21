#pragma once

#include "TasPlayer.hpp"

struct TasFramebulk;

struct TasToolParams {
    std::vector<std::string> raw;
    TasToolParams() {}
    TasToolParams(std::vector<std::string> rawParams) : raw(rawParams) {}
};


class TasTool {
protected:
    const char* name;
    std::shared_ptr<TasToolParams> params = nullptr;
    bool updated = false;
public:
    TasTool(const char* name);
    ~TasTool();

    const char* GetName();
    
    virtual TasTool* GetTool() = 0;
    virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>) = 0;
    virtual void Apply(TasFramebulk& fb) = 0;
    virtual void Reset();

    void SetParams(std::shared_ptr<TasToolParams> params);
    std::shared_ptr<TasToolParams> GetCurrentParams();

public:
    static std::vector<TasTool*>& GetList();
};


class TasToolCommand {
public:
    TasTool* tool;
    std::shared_ptr<TasToolParams> params;

    TasToolCommand(TasTool* tool, std::shared_ptr<TasToolParams> params)
        : tool(tool)
        , params(params)
    {}

    ~TasToolCommand() {}
};