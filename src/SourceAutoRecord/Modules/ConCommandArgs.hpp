#pragma once
#include "../Offsets.hpp"
#include "Tier1.hpp"

namespace Tier1
{
	struct ConCommandArgs {
		const void* Ptr;

		ConCommandArgs::ConCommandArgs(const void* ptr) : Ptr(ptr) {
		}
		int ConCommandArgs::Count() const {
			switch (Offsets::Variant) {
			case 0:	// Portal 2 6879
			case 1: // INFRA 6905
				//return ((const ConCommandArgsData*)Ptr)->ArgC;
				auto ptr = (const ConCommandArgsData*)Ptr;
				auto test = *ptr;
				return ptr->ArgC;
			}
			return 0;
		}
		const char* ConCommandArgs::At(int index) const {
			switch (Offsets::Variant) {
			case 0:	// Portal 2 6879
			case 1: // INFRA 6905
				return ((const ConCommandArgsData*)Ptr)->ArgV[index];
			}
			return nullptr;
		}
		const char* ConCommandArgs::FullArgs() const {
			switch (Offsets::Variant) {
			case 0:	// Portal 2 6879
			case 1: // INFRA 6905
				return ((const ConCommandArgsData*)Ptr)->ArgSBuffer;
			}
			return nullptr;
		}
	};
}