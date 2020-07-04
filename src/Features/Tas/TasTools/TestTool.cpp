#include "TestTool.hpp"

TestTool tasTestTool("testtool");

void TestTool::Apply(TasFramebulk& fb)
{
    auto ttParams = std::static_pointer_cast<TestToolParams>(params);

    fb.viewAnalog.y += sinf(fb.tick / 10.0f) * ttParams->force;
}

std::shared_ptr<TasToolParams> TestTool::ParseParams(std::vector<std::string>) {
    return std::make_shared<TestToolParams>();
}

void TestTool::Reset()
{
    params = std::make_shared<TestToolParams>();
}