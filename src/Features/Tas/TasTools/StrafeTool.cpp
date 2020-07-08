#include "StrafeTool.hpp"
#include "../TasParser.hpp"

AutoStrafeTool autoStrafeTool("strafe");

void AutoStrafeTool::Apply(TasFramebulk& fb)
{
    auto asParams = std::static_pointer_cast<AutoStraferParams>(params);

    if (asParams->strafeDir.type == CURRENT && !asParams->strafeDir.processed) {
        asParams->strafeDir.angle = 0;
        asParams->strafeDir.processed = true;
    }
}

AutoStrafeTool* AutoStrafeTool::GetTool()
{
    return &autoStrafeTool;
}

std::shared_ptr<TasToolParams> AutoStrafeTool::ParseParams(std::vector<std::string> vp)
{
    if (vp.size() < 1) {
        return nullptr;
    }

    AutoStraferType type;
    //Type : off, normal, vectorial
    if (vp[0] == "off") {
        type = AutoStraferType::NONE;
    } else if (vp[0] == "normal") {
        type = AutoStraferType::ANGULAR;
    } else {
        type = AutoStraferType::VECTORIAL;
    }

    AutoStraferDirection dir{ AutoStraferDirectionType::CURRENT, 0 };
    //Direction : left, right, or angle
    if (vp.size() > 1) {
        float angle;
        if (TasParser::isNumber(vp[1])) {
            dir.angle = TasParser::toFloat(vp[1]);
        } else {
            if (vp[1] == "left") {
                dir.type = AutoStraferDirectionType::LEFT;
            } else {
                dir.type = AutoStraferDirectionType::RIGHT;
            }
        }
    }

    AutoStraferSpeed speed{ true, 0 };
    if (vp.size() > 2) {
        float f;
        if (TasParser::isNumber(vp[2])) {
            speed.max = false;
            speed.speed = TasParser::toFloat(vp[2]);
        } else {
            speed.max = true;
        }
    }

    return std::make_shared<AutoStraferParams>(type, dir, speed);
}

void AutoStrafeTool::Reset()
{
    params = std::make_shared<AutoStraferParams>();
}
