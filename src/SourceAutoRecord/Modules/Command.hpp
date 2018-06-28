#pragma once
#include "Tier1.hpp"

namespace Tier1
{
	struct Command {
		void* ptr = nullptr;
		std::unique_ptr<uint8_t[]> data;

        Command()
        {
        }
		Command(const char* name, _CommandCallback callback, const char* helpstr = "", int flags = 0)
        {
			auto size = sizeof(ConCommand);
			data = std::make_unique<uint8_t[]>(size);
			this->ptr = data.get();
			std::memset(this->ptr, 0, size);

            ConCommandCtor(this->ptr, name, (void*)callback, helpstr, flags, nullptr);
		    ConCommandCount++;
		}
        Command(const char* name, _CommandCallbackArgs callback, const char* helpstr = "", int flags = 0)
        {
			auto size = sizeof(ConCommand);
			data = std::make_unique<uint8_t[]>(size);
			this->ptr = data.get();
			std::memset(this->ptr, 0, size);

            ConCommandCtor2(this->ptr, name, (void*)callback, helpstr, flags, nullptr);
		    ConCommandCount++;
		}
	};
}