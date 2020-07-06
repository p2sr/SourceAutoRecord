#include "AutoStrafeTool.hpp"

#include "Modules/Engine.hpp"

AutoStrafeTool autoStrafeTool("strafe");

void AutoStrafeTool::Apply(TasFramebulk& fb)
{
    auto asParams = std::static_pointer_cast<AutoStraferParams>(params);
    
    if (asParams->strafeDir.type == CURRENT && !asParams->strafeDir.processed) {
        asParams->strafeDir.angle = engine->GetAngles(GET_SLOT()).y;
        asParams->strafeDir.processed = true;
    }
}

std::shared_ptr<TasToolParams> AutoStrafeTool::ParseParams(std::vector<std::string>)
{
    return std::make_shared<AutoStraferParams>();
}

void AutoStrafeTool::Reset()
{
    params = std::make_shared<AutoStraferParams>();
}