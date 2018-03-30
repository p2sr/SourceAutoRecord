#pragma once
#include "Tier1.hpp"

#include "Game.hpp"
#include "SourceAutoRecord.hpp"

namespace Tier1
{
	_ConCommand ConCommandCtor;
	_ConCommand ConCommandCtor2;

	struct ConCommandData : ConCommandBase {
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

	bool ConCommandLoaded()
	{
		auto cnc = SAR::Find("ConCommand_Ctor1");
		auto cnc2 = SAR::Find("ConCommand_Ctor2");
		if (cnc.Found && cnc2.Found) {
			ConCommandCtor = reinterpret_cast<_ConCommand>(cnc.Address);
			ConCommandCtor2 = reinterpret_cast<_ConCommand>(cnc2.Address);
		}
		return cnc.Found && cnc2.Found;
	}

	struct ConCommand {
		void* Ptr = nullptr;
		std::unique_ptr<uint8_t[]> Blob;

		ConCommand() {
			size_t size = sizeof(ConCommandData);

			Blob = std::make_unique<uint8_t[]>(size);
			Ptr = Blob.get();
			std::memset(Ptr, 0, size);
		}
	};

	ConCommand CreateCommand(const char* name, _CommandCallbackVoid callback, const char* helpstr = "", int flags = 0)
	{
		auto cc = ConCommand();
		ConCommandCtor(cc.Ptr, name, (void*)callback, helpstr, flags, nullptr);
		ConCommandCount++;
		return cc;
	}
	ConCommand CreateCommandArgs(const char* name, _CommandCallbackArgs callback, const char* helpstr = "", int flags = 0)
	{
		auto cc = ConCommand();
		ConCommandCtor2(cc.Ptr, name, (void*)callback, helpstr, flags, nullptr);
		ConCommandCount++;
		return cc;
	}
}