#include "AutoJumpTool.hpp"

#include "Modules/Server.hpp"
#include "Modules/Engine.hpp"

AutoJumpTool autoJumpTool("autojump");

void AutoJumpTool::Apply(TasFramebulk& bulk)
{
    auto ttParams = std::static_pointer_cast<AutoJumpToolParams>(this->params);
    if (ttParams->enabled) {

        auto player = server->GetPlayer(GET_SLOT() + 1);
        if (!player) {
            return;
        }
        unsigned int groundEntity = *reinterpret_cast<unsigned int*>((uintptr_t)player + 344); // m_hGroundEntity
        bool grounded = groundEntity != 0xFFFFFFFF;

        if (grounded) {
            bulk.buttonStates[TasControllerInput::Jump] = true;
        } else {
            bulk.buttonStates[TasControllerInput::Jump] = false;
        }
    }
}

AutoJumpTool* AutoJumpTool::GetTool()
{
    return &autoJumpTool;
}

std::shared_ptr<TasToolParams> AutoJumpTool::ParseParams(std::vector<std::string> vp)
{
    if (vp.empty())
        return nullptr;

    bool arg = vp[0] == "on";

    return std::make_shared<AutoJumpToolParams>(arg);
}

void AutoJumpTool::Reset()
{
    this->params = std::make_shared<AutoJumpToolParams>();
}
