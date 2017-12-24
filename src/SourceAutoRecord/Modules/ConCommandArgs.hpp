#pragma once
#include "Tier1.hpp"

#include "Offsets.hpp"

namespace Tier1
{
	struct ConCommandArgs {
		const void* Ptr;

		ConCommandArgs::ConCommandArgs(const void* ptr) : Ptr(ptr) {
		}
		int ConCommandArgs::Count() const {
			return ((const ConCommandArgsData*)Ptr)->ArgC;
		}
		const char* ConCommandArgs::At(int index) const {
			return ((const ConCommandArgsData*)Ptr)->ArgV[index];
		}
		const char* ConCommandArgs::FullArgs() const {
			return ((const ConCommandArgsData*)Ptr)->ArgSBuffer;
		}
	};
}