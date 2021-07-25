#include "SetAngleTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

struct SetAngleParams : public TasToolParams {
    SetAngleParams()
        : TasToolParams()
    { }

    SetAngleParams(int ticks, float pitch, float yaw)
        : TasToolParams(true)
        , ticks(ticks)
        , elapsed(0)
        , pitch(pitch)
        , yaw(yaw)
    { }

    int ticks;
    int elapsed;
    float pitch;
    float yaw;
};

SetAngleTool setAngleTool;

SetAngleTool* SetAngleTool::GetTool()
{
    return &setAngleTool;
}

void SetAngleTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &playerInfo)
{
    auto params = std::static_pointer_cast<SetAngleParams>(this->params);
    
    if (!params->enabled) {
        return;
    }

    if (params->elapsed >= params->ticks) {
        params->enabled = false;
        return;
    }

    int remaining = params->ticks - params->elapsed;

    Vector requiredDelta = QAngleToVector(playerInfo.angles) - Vector{ params->pitch, params->yaw };

    float pitchDelta = requiredDelta.x / remaining;
    float yawDelta = requiredDelta.y / remaining;

    bulk.viewAnalog = bulk.viewAnalog - Vector{ -yawDelta, -pitchDelta };

    if (sar_tas_debug.GetBool()) {
        console->Print("setang %.3f %.3f\n", bulk.viewAnalog.x, bulk.viewAnalog.y);
    }

    ++params->elapsed;
}

std::shared_ptr<TasToolParams> SetAngleTool::ParseParams(std::vector<std::string> vp)
{
    if (vp.size() != 2 && vp.size() != 3) {
        return nullptr;
    }

    float pitch = atof(vp[0].c_str());
    float yaw = atof(vp[1].c_str());

    int ticks = vp.size() == 3 ? atoi(vp[2].c_str()) : 1;

    return std::make_shared<SetAngleParams>(ticks, pitch, yaw);
}

void SetAngleTool::Reset()
{
    params = std::make_shared<SetAngleParams>();
}
