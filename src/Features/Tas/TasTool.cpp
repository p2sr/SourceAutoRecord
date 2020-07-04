#include "TasTool.hpp"

#include "TasPlayer.hpp"

std::vector<TasTool*>& TasTool::GetList()
{
    static std::vector<TasTool*> list;
    return list;
}

TasTool::TasTool(const char* name) : name(name) {
    this->GetList().push_back(this);
}

TasTool::~TasTool()
{

}

void TasTool::SetParams(std::shared_ptr<TasToolParams> params)
{
    this->params = params;
}

void TasTool::Reset() {
    params = std::make_shared<TasToolParams>();
}

std::shared_ptr<TasToolParams> TasTool::GetCurrentParams()
{
    return params;
}