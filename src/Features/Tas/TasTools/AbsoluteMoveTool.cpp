#include "AbsoluteMoveTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

AbsoluteMoveTool tasAbsoluteMoveTool("absmov");

AbsoluteMoveTool* AbsoluteMoveTool::GetTool()
{
    return &tasAbsoluteMoveTool;
}

void AbsoluteMoveTool::Apply(TasFramebulk& fb, const TasPlayerInfo& pInfo)
{
    auto ttParams = std::static_pointer_cast<AbsoluteMoveToolParams>(params);
    
    if (!ttParams->enabled)
        return;

    auto nSlot = GET_SLOT();

    float angle = pInfo.angles.y - fb.viewAnalog.x - 90.0;
    float desired = ttParams->direction;

    float delta = desired - angle;
    auto R = DEG2RAD(delta);
    auto X = cosf(R);
    auto Y = sinf(R);

    fb.moveAnalog.x = X;
    fb.moveAnalog.y = Y;

    if (sar_tas_debug.GetBool()) {
        console->Print("absmov %.3f %.3f\n", X, Y);
    }
}

std::shared_ptr<TasToolParams> AbsoluteMoveTool::ParseParams(std::vector<std::string> vp)
{
    if (vp.empty())
        return nullptr;

    if (vp[0] == "off")
        return std::make_shared<AbsoluteMoveToolParams>();

   return std::make_shared<AbsoluteMoveToolParams>(std::stof(vp[0]));
}

void AbsoluteMoveTool::Reset()
{
    params = std::make_shared<AbsoluteMoveToolParams>();
}
