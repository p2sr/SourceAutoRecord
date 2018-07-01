#pragma once
#include "Tier1.hpp"

namespace Tier1 {

struct Command {
    void* ptr = nullptr;
    std::unique_ptr<uint8_t[]> data;

    Command()
    {
    }
    Command(const char* pName, _CommandCallback callback, const char* pHelpString, int flags = 0, _CommandCallbackCompletion completionFunc = nullptr)
    {
        auto size = sizeof(ConCommand);
        data = std::make_unique<uint8_t[]>(size);
        this->ptr = data.get();
        std::memset(this->ptr, 0, size);

        Original::ConCommandCtor(this->ptr, pName, (void*)callback, pHelpString, flags, (int*)completionFunc);
        ConCommandCount++;
    }
    Command(const char* pName, _CommandCallbackArgs callback, const char* pHelpString, int flags = 0, _CommandCallbackCompletion completionFunc = nullptr)
    {
        auto size = sizeof(ConCommand);
        data = std::make_unique<uint8_t[]>(size);
        this->ptr = data.get();
        std::memset(this->ptr, 0, size);

        Original::ConCommandCtor2(this->ptr, pName, (void*)callback, pHelpString, flags, (int*)completionFunc);
        ConCommandCount++;
    }
};
}