#include "Tier1.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

struct ConFilterRule {
	bool allow;
	std::string str;
};

static Variable sar_con_filter("sar_con_filter", "0", "Enable the console filter\n");
static Variable sar_con_filter_default("sar_con_filter_default", "0", "Whether to allow text through the console filter by default\n");
static std::vector<ConFilterRule> g_con_filter_rules;

CON_COMMAND(sar_con_filter_allow, "sar_con_filter_allow <string> - add an allow rule to the console filter\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_con_filter_allow.ThisPtr()->m_pszHelpString);
	}
	g_con_filter_rules.push_back({true, args[1]});
}

CON_COMMAND(sar_con_filter_block, "sar_con_filter_block <string> - add a disallow rule to the console filter\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_con_filter_block.ThisPtr()->m_pszHelpString);
	}
	g_con_filter_rules.push_back({false, args[1]});
}

CON_COMMAND(sar_con_filter_reset, "sar_con_filter_reset - clear the console filter rule list\n") {
	if (args.ArgC() != 1) {
		return console->Print(sar_con_filter_reset.ThisPtr()->m_pszHelpString);
	}
	g_con_filter_rules.clear();
}

class ConsoleDisplayHook : public IConsoleDisplayFunc {
public:
	virtual void ColorPrint(const Color &clr, const char *msg) override {
		if (!MatchesFilters(msg)) return;
		tier1->orig_display_func->ColorPrint(clr, msg);
	}
	virtual void Print(const char *msg) override {
		if (!MatchesFilters(msg)) return;
		tier1->orig_display_func->Print(msg);
	}
	virtual void DPrint(const char *msg) override {
		if (!MatchesFilters(msg)) return;
		tier1->orig_display_func->DPrint(msg);
	}
	virtual void GetConsoleText(char *text, size_t bufSize) const override {
		tier1->orig_display_func->GetConsoleText(text, bufSize);
	}

private:
	bool MatchesFilters(const char *msg) {
		if (!sar_con_filter.GetBool()) return true;

		for (auto &rule : g_con_filter_rules) {
			if (strstr(msg, rule.str.c_str())) {
				return rule.allow;
			}
		}

		return sar_con_filter_default.GetBool();
	}
};

static ConsoleDisplayHook g_con_display_hook;

bool Tier1::Init() {
	this->g_pCVar = Interface::Create(this->Name(), "VEngineCvar007", false);
	if (this->g_pCVar) {
		this->RegisterConCommand = this->g_pCVar->Original<_RegisterConCommand>(Offsets::RegisterConCommand);
		this->UnregisterConCommand = this->g_pCVar->Original<_UnregisterConCommand>(Offsets::UnregisterConCommand);
		this->FindCommandBase = this->g_pCVar->Original<_FindCommandBase>(Offsets::FindCommandBase);

		this->m_pConCommandList = *(ConCommandBase **)((uintptr_t)this->g_pCVar->ThisPtr() + Offsets::m_pConCommandList);
		this->m_DisplayFuncs = (CUtlVector<IConsoleDisplayFunc *> *)((uintptr_t)this->g_pCVar->ThisPtr() + Offsets::m_DisplayFuncs);

		if (this->m_DisplayFuncs->m_Size == 1) {
			// that callback writes to the console - store and hook it
			this->orig_display_func = this->m_DisplayFuncs->m_pElements[0];
			this->m_DisplayFuncs->m_pElements[0] = &g_con_display_hook;
		}

		auto listdemo = reinterpret_cast<ConCommand *>(this->FindCommandBase(this->g_pCVar->ThisPtr(), "listdemo"));
		if (listdemo) {
			this->ConCommand_VTable = *(void **)listdemo;

			if (listdemo->m_fnCompletionCallback) {
				auto callback = (uintptr_t)listdemo->m_fnCompletionCallback + Offsets::AutoCompletionFunc;
				this->AutoCompletionFunc = Memory::Read<_AutoCompletionFunc>(callback);
			}
		}

		auto sv_lan = reinterpret_cast<ConVar *>(this->FindCommandBase(this->g_pCVar->ThisPtr(), "sv_lan"));
		if (sv_lan) {
			this->ConVar_VTable = *(void **)sv_lan;
			this->ConVar_VTable2 = sv_lan->ConVar_VTable;

			auto vtable =
#ifdef _WIN32
				&this->ConVar_VTable2;
#else
				&this->ConVar_VTable;
#endif

			this->Dtor = Memory::VMT<_Dtor>(vtable, Offsets::Dtor);
			this->Create = Memory::VMT<_Create>(vtable, Offsets::Create);
		}

		this->InstallGlobalChangeCallback = this->g_pCVar->Original<_InstallGlobalChangeCallback>(Offsets::InstallGlobalChangeCallback);
		this->RemoveGlobalChangeCallback = this->g_pCVar->Original<_RemoveGlobalChangeCallback>(Offsets::RemoveGlobalChangeCallback);
	}

	return this->hasLoaded = this->g_pCVar && this->ConCommand_VTable && this->ConVar_VTable && this->ConVar_VTable2 && this->AutoCompletionFunc;
}
void Tier1::Shutdown() {
	if (this->orig_display_func) {
		this->m_DisplayFuncs->m_pElements[0] = this->orig_display_func;
	}
	Interface::Delete(this->g_pCVar);
}

Tier1 *tier1;

CBaseAutoCompleteFileList::CBaseAutoCompleteFileList(const char *cmdname, const char *subdir, const char *extension) {
	m_pszCommandName = cmdname;
	m_pszSubDir = subdir;
	m_pszExtension = extension;
}
int CBaseAutoCompleteFileList::AutoCompletionFunc(char const *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	return tier1->AutoCompletionFunc(this, partial, commands);
}
