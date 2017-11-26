#pragma once
#include "../SourceAutoRecord.h"

#include "../Offsets.h"
#include "../Utils.h"

#include "Tier1.h"

namespace Tier1
{
	_ConCommand ConCommandCtor;

	// Portal 2 6879
	// INFRA 6905
	struct ConCommandData0 : ConCommandBase {
		union {
			void* CommandCallbackV1;
			void* CommandCallback;
			void* CommandCallbackInterface;
		};
		union {
			void* CompletionCallback;
			void* CommandCompletionCallback;
		};
		bool HasCompletionCallback : 1;
		bool UsingNewCommandCallback : 1;
		bool UsingCommandCallbackInterface : 1;
	};

	void InitConCommand(uintptr_t conCommandAddr)
	{
		ConCommandCtor = reinterpret_cast<_ConCommand>(conCommandAddr);
	}

	struct ConCommand {
		void* Ptr = nullptr;
		std::unique_ptr<uint8_t[]> Blob;

		ConCommand::ConCommand()
		{
			size_t size = 0;

			switch (Offsets::Variant) {
			case 0:
			case 1:
				size = sizeof(ConCommandData0);
				break;
			}

			Blob = std::make_unique<uint8_t[]>(size);
			Ptr = Blob.get();
			std::memset(Ptr, 0, size);
		}
	};

	ConCommand CreateCommand(const char* name, _CommandCallbackVoid callback, const char* helpstr = "")
	{
		auto ret = ConCommand();
		ConCommandCtor(ret.Ptr, nullptr, name, callback, helpstr, 0, nullptr);
		return ret;
	}
}