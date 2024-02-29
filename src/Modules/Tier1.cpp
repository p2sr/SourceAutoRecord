#include "Tier1.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"
#include "Event.hpp"

struct ConFilterRule {
	bool allow;
	std::string str;
	std::string end;
};

static Variable sar_con_filter("sar_con_filter", "0", "Enable the console filter\n");
static Variable sar_con_filter_default("sar_con_filter_default", "0", "Whether to allow text through the console filter by default\n");
static Variable sar_con_filter_suppress_blank_lines("sar_con_filter_suppress_blank_lines", "0", "Whether to suppress blank lines in console\n");
static std::vector<ConFilterRule> g_con_filter_rules;

CON_COMMAND(sar_con_filter_allow, "sar_con_filter_allow <string> [end] - add an allow rule to the console filter, allowing until 'end' is matched\n") {
	if (args.ArgC() != 2 && args.ArgC() != 3) {
		return console->Print(sar_con_filter_allow.ThisPtr()->m_pszHelpString);
	}
	g_con_filter_rules.push_back({true, args[1], args.ArgC() == 3 ? args[2] : ""});
}

CON_COMMAND(sar_con_filter_block, "sar_con_filter_block <string> [end] - add a disallow rule to the console filter, blocking until 'end' is matched\n") {
	if (args.ArgC() != 2 && args.ArgC() != 3) {
		return console->Print(sar_con_filter_block.ThisPtr()->m_pszHelpString);
	}
	g_con_filter_rules.push_back({false, args[1], args.ArgC() == 3 ? args[2] : ""});
}

struct BufferedPart {
	enum class Type {
		COL_PRINT,
		PRINT,
		DPRINT,
	};
	Type type;
	std::string str;
	Color col = Color();
};

class ConsoleDisplayHook : public IConsoleDisplayFunc {
public:
	virtual void ColorPrint(const Color &clr, const char *msg) override {
		AddToBuffer(msg, BufferedPart::Type::COL_PRINT, clr);
	}
	virtual void Print(const char *msg) override {
		AddToBuffer(msg, BufferedPart::Type::PRINT, Color());
	}
	virtual void DPrint(const char *msg) override {
		AddToBuffer(msg, BufferedPart::Type::DPRINT, Color());
	}
	virtual void GetConsoleText(char *text, size_t bufSize) const override {
		tier1->orig_display_func->GetConsoleText(text, bufSize);
	}

	void AddToBuffer(const char *msg, BufferedPart::Type type, Color col) {
		auto lines = GetLines(msg);
		for (size_t i = 0; i < lines.size() - 1; ++i) {
			this->buf.push_back({
				type,
				lines[i],
				col,
			});
			this->Flush();
		}
		// If there's a trailing bit, add it in to the buffer
		if (lines[lines.size() - 1].size() != 0) {
			this->buf.push_back({
				type,
				lines[lines.size() - 1],
				col,
			});
		}
	}

	void Flush() {
		// Flush any un-written string

		std::string str;
		for (auto &b : this->buf) {
			str += b.str;
		}

		if (str.size() == 0) {
			this->buf.clear();
			return;
		}

		if (MatchesFilters(str.c_str())) {
			if (!IsNewline(str.c_str()) || !this->last_was_newline || !sar_con_filter_suppress_blank_lines.GetBool()) {
				for (auto &b : this->buf) {
					switch (b.type) {
					case BufferedPart::Type::COL_PRINT:
						tier1->orig_display_func->ColorPrint(b.col, b.str.c_str());
						break;
					case BufferedPart::Type::PRINT:
						tier1->orig_display_func->Print(b.str.c_str());
						break;
					case BufferedPart::Type::DPRINT:
						tier1->orig_display_func->DPrint(b.str.c_str());
						break;
					}
				}
				this->last_was_newline = str[str.size() - 1] == '\n';
			}
		}

		this->buf.clear();
	}

	void ResetFilters() {
		g_con_filter_rules.clear();
		this->do_until_rule = {};
	}

	void DebugFilters() {
		bool filter = sar_con_filter.GetBool();
		sar_con_filter.SetValue("0");
		console->Print("Filter rules:\n");
		for (auto &rule : g_con_filter_rules) {
			console->Print("(%s) \"%s\" \"%s\"\n", rule.allow ? "allow" : "block", rule.str.c_str(), rule.end.c_str());
		}
		if (this->do_until_rule.has_value()) {
			auto rule = this->do_until_rule.value();
			console->Print("current rule: (%s) \"%s\" \"%s\"\n", rule.allow ? "allow" : "block", rule.str.c_str(), rule.end.c_str());
		}
		sar_con_filter.SetValue(filter ? "1" : "0");
	}

private:
	bool IsNewline(const char *str) {
		while (*str == ' ') ++str;
		return *str == '\n';
	}

	bool MatchesFilters(const char *msg) {
		if (!sar_con_filter.isRegistered || !sar_con_filter.GetBool()) return true;

		if (this->do_until_rule.has_value()) {
			bool match = this->do_until_rule->allow;
			if (MatchesPattern(msg, this->do_until_rule->end.c_str())) {
				this->do_until_rule = {};
			}
			return match;
		}

		for (auto &rule : g_con_filter_rules) {
			if (MatchesPattern(msg, rule.str.c_str())) {
				if (!MatchesPattern(msg, rule.end.c_str())) {
					this->do_until_rule = rule;
				}
				return rule.allow;
			}
		}

		return sar_con_filter_default.GetBool();
	}

	bool MatchesPattern(const char *str, const std::string &pat) {
		size_t len = strlen(str);

		if (pat.size() == 0) return true;
		if (!*str) return false;

		if (pat[0] == '^' && pat[pat.size()-1] == '$') {
			if (str[len-1] != '\n') return false;
			return pat.substr(1, pat.size()-2) == std::string(str, len-1);
		}

		if (pat[0] == '^') {
			return Utils::StartsWith(str, pat.c_str() + 1);
		}

		if (pat[pat.size()-1] == '$') {
			if (str[len-1] != '\n') return false;
			auto suf = pat.substr(0, pat.size() - 1);
			return Utils::EndsWith(std::string(str, len-1), suf);
		}

		return strstr(str, pat.c_str());
	}

	std::vector<std::string> GetLines(const char *msg) {
		std::vector<std::string> vec;
		const char *start = msg;
		for (const char *cur = msg; *cur; ++cur) {
			if (*cur == '\n') {
				vec.push_back({start, (unsigned)(cur + 1 - start)});
				start = cur + 1;
			}
		}
		vec.push_back({start});
		return vec;
	}

	std::vector<BufferedPart> buf;
	std::optional<ConFilterRule> do_until_rule;
	bool last_was_newline;
};

static ConsoleDisplayHook g_con_display_hook;

ON_EVENT(FRAME) {
	// Flush any un-written string since we haven't received anything
	// terminating the line this frame
	if (tier1->orig_display_func) g_con_display_hook.Flush();
}

CON_COMMAND(sar_con_filter_reset, "sar_con_filter_reset - clear the console filter rule list\n") {
	if (args.ArgC() != 1) {
		return console->Print(sar_con_filter_reset.ThisPtr()->m_pszHelpString);
	}
	g_con_display_hook.ResetFilters();
}

CON_COMMAND(sar_con_filter_debug, "sar_con_filter_debug - print the console filter rule list\n") {
	if (args.ArgC() != 1) {
		return console->Print(sar_con_filter_debug.ThisPtr()->m_pszHelpString);
	}
	g_con_display_hook.DebugFilters();
}

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

		auto tier1 = Memory::GetModuleHandleByName(this->Name());
		if (tier1) {
			this->KeyValuesSystem = Memory::GetSymbolAddress<_KeyValuesSystem>(tier1, "KeyValuesSystem");
		}
	}

	return this->hasLoaded = this->g_pCVar && this->ConCommand_VTable && this->ConVar_VTable && this->ConVar_VTable2;
}
void Tier1::Shutdown() {
	if (this->orig_display_func) {
		this->m_DisplayFuncs->m_pElements[0] = this->orig_display_func;
	}
	Interface::Delete(this->g_pCVar);
}

Tier1 *tier1;
