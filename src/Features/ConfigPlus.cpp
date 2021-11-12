#include "Event.hpp"
#include "Features/Session.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"

#include <cstdlib>
#include <cstring>
#include <map>
#include <queue>
#include <stack>
#include <vector>

// Fuck you Windows
#ifdef _WIN32
#	define strdup _strdup
#endif

static std::map<std::string, std::string> g_svars;

static std::string GetSvar(std::string name) {
	auto it = g_svars.find(name);
	if (it == g_svars.end()) return "";
	return it->second;
}

CON_COMMAND_F(svar_set, "svar_set <variable> <value> - set a svar (SAR variable) to a given value\n", FCVAR_DONTRECORD) {
	if (args.ArgC() != 3) {
		return console->Print(svar_set.ThisPtr()->m_pszHelpString);
	}

	g_svars[std::string(args[1])] = args[2];
}

CON_COMMAND_F(svar_get, "svar_get <variable> - get the value of a svar\n", FCVAR_DONTRECORD) {
	if (args.ArgC() != 2) {
		return console->Print(svar_get.ThisPtr()->m_pszHelpString);
	}

	console->Print("%s = %s\n", args[1], GetSvar({args[1]}).c_str());
}

CON_COMMAND_F(svar_count, "svar_count - prints a count of all the defined svars\n", FCVAR_DONTRECORD) {
	if (args.ArgC() != 1) {
		return console->Print(svar_count.ThisPtr()->m_pszHelpString);
	}

	int size = g_svars.size();

	console->Print("%d svar defined\n", size);
}

static ConsoleListener *g_svarListener;
static std::string g_svarListenerTarget;
static std::string g_svarListenerOutput;

CON_COMMAND_F(_sar_svar_capture_stop, "Internal SAR command. Do not use\n", FCVAR_DONTRECORD | FCVAR_HIDDEN) {
	delete g_svarListener;
	g_svarListener = nullptr;

	_sar_svar_capture_stop.ThisPtr()->m_nFlags |= FCVAR_HIDDEN;

	std::string out = g_svarListenerOutput;
	out.erase(std::remove(out.begin(), out.end(), '\n'), out.end());

	g_svars[g_svarListenerTarget] = out;
}

CON_COMMAND_F(svar_capture, "svar_capture <variable> <command> [args]... - capture a command's output and place it into an svar, removing newlines\n", FCVAR_DONTRECORD) {
	if (args.ArgC() < 3) {
		return console->Print(svar_capture.ThisPtr()->m_pszHelpString);
	}

	const char *cmd;

	if (args.ArgC() == 3) {
		cmd = args[2];
	} else {
		cmd = args.m_pArgSBuffer + args.m_nArgv0Size;

		while (isspace(*cmd)) ++cmd;

		if (*cmd == '"') {
			cmd += strlen(args[1]) + 2;
		} else {
			cmd += strlen(args[1]);
		}

		while (isspace(*cmd)) ++cmd;
	}

	g_svarListenerTarget = args[1];
	g_svarListenerOutput = "";

	g_svarListener = new ConsoleListener([&](const char *msg) {
		g_svarListenerOutput += msg;
	});

	_sar_svar_capture_stop.ThisPtr()->m_nFlags &= ~FCVAR_HIDDEN;

	// The engine won't let us execute a command during execution of
	// this one; instead, they're added to the front of the cbuf, to be
	// executed next. So we have to do this jank to get the output
	// properly
	engine->ExecuteCommand(cmd, true);
	engine->ExecuteCommand("_sar_svar_capture_stop", true);
}

CON_COMMAND_F(svar_from_cvar, "svar_from_cvar <variable> <cvar> - capture a cvars's value and place it into an svar, removing newlines\n", FCVAR_DONTRECORD) {
	if (args.ArgC() != 3) {
		return console->Print(svar_from_cvar.ThisPtr()->m_pszHelpString);
	}

	Variable cvar(args[2]);

	if (cvar.ThisPtr()) {
		std::string val = cvar.GetFlags() & FCVAR_NEVER_AS_STRING ? std::to_string(cvar.GetInt()) : cvar.GetString();
		val.erase(std::remove(val.begin(), val.end(), '\n'), val.end());
		g_svars[std::string(args[1])] = val;
	}
}

#define SVAR_OP(name, op, disallowSecondZero)                                                                                            \
	CON_COMMAND_F(svar_##name, "svar_" #name " <variable> <variable|value> - perform the given operation on an svar\n", FCVAR_DONTRECORD) { \
		if (args.ArgC() != 3) {                                                                                                                \
			return console->Print(svar_##name.ThisPtr()->m_pszHelpString);                                                                        \
		}                                                                                                                                      \
                                                                                                                                         \
		int cur;                                                                                                                               \
                                                                                                                                         \
		{                                                                                                                                      \
			auto it = g_svars.find({args[1]});                                                                                                    \
			if (it == g_svars.end()) {                                                                                                            \
				cur = 0;                                                                                                                             \
			} else {                                                                                                                              \
				cur = atoi(it->second.c_str());                                                                                                      \
			}                                                                                                                                     \
		}                                                                                                                                      \
                                                                                                                                         \
		char *end;                                                                                                                             \
		int other = strtol(args[2], &end, 10);                                                                                                 \
                                                                                                                                         \
		if (end == args[2] || *end) {                                                                                                          \
			auto it = g_svars.find({args[2]});                                                                                                    \
			if (it == g_svars.end()) {                                                                                                            \
				other = 0;                                                                                                                           \
			} else {                                                                                                                              \
				other = atoi(it->second.c_str());                                                                                                    \
			}                                                                                                                                     \
		}                                                                                                                                      \
                                                                                                                                         \
		int val = (disallowSecondZero && other == 0) ? 0 : cur op other;                                                                       \
		g_svars[std::string(args[1])] = Utils::ssprintf("%d", val);                                                                            \
	}

SVAR_OP(add, +, false)
SVAR_OP(sub, -, false)
SVAR_OP(mul, *, false)
SVAR_OP(div, /, true)
SVAR_OP(mod, %, true)

struct Condition {
	enum {
		ORANGE,
		COOP,
		CM,
		SAME_MAP,
		WORKSHOP,
		MENU,
		MAP,
		PREV_MAP,
		GAME,
		NOT,
		AND,
		OR,
		SVAR,
	} type;

	union {
		char *map; // also actually used for game name now lol
		Condition *unop_cond;
		struct {
			Condition *binop_l, *binop_r;
		};
		struct {
			char *var, *val;
		} svar;
	};
};

static void FreeCondition(Condition *c) {
	switch (c->type) {
	case Condition::MAP:
	case Condition::PREV_MAP:
	case Condition::GAME:
		free(c->map);
		break;
	case Condition::SVAR:
		free(c->svar.var);
		free(c->svar.val);
		break;
	case Condition::NOT:
		FreeCondition(c->unop_cond);
		break;
	case Condition::AND:
	case Condition::OR:
		FreeCondition(c->binop_l);
		FreeCondition(c->binop_r);
		break;
	default:
		break;
	}

	free(c);
}

static bool isSpeedrunMod() {
	static bool checked = false;
	static bool srm = false;

	if (!checked) {
		auto serverPlugin = (uintptr_t)(engine->s_ServerPlugin->ThisPtr());
		auto count = *(int *)(serverPlugin + 16); // CServerPlugin::m_Size
		if (count > 0) {
			auto plugins = *(uintptr_t *)(serverPlugin + 4); // CServerPlugin::m_Plugins
			for (size_t i = 0; i < count; ++i) {
				auto ptr = *(CPlugin **)(plugins + i * sizeof (uintptr_t));
				if (!strcmp(ptr->m_szName, "Speedrun Mod was a mistake.")) {
					srm = true;
					break;
				}
			}
		}

		checked = true;
	}

	return srm;
}

static const char *gameName() {
	switch (sar.game->GetVersion()) {
	case SourceGame_ApertureTag:
		return "aptag";
	case SourceGame_PortalStoriesMel:
		return "mel";
	case SourceGame_ThinkingWithTimeMachine:
		return "twtm";
	case SourceGame_PortalReloaded:
		return "reloaded";
	case SourceGame_Portal2:
		return isSpeedrunMod() ? "srm" : "portal2";
	default:
		return "other";
	}
}

static bool EvalCondition(Condition *c) {
	switch (c->type) {
	case Condition::ORANGE: return engine->IsOrange();
	case Condition::COOP: return engine->IsCoop();
	case Condition::CM: return client->GetChallengeStatus() == CMStatus::CHALLENGE;
	case Condition::SAME_MAP: return session->previousMap == engine->GetCurrentMapName();
	case Condition::WORKSHOP: return !strncmp("workshop/", engine->GetCurrentMapName().c_str(), 9);
	case Condition::MENU: return engine->GetCurrentMapName().size() == 0;
	case Condition::MAP: return !strcmp(c->map, engine->GetCurrentMapName().c_str());
	case Condition::PREV_MAP: return !strcmp(c->map, session->previousMap.c_str());
	case Condition::GAME: return !strcmp(c->map, gameName());
	case Condition::NOT: return !EvalCondition(c->unop_cond);
	case Condition::AND: return EvalCondition(c->binop_l) && EvalCondition(c->binop_r);
	case Condition::OR: return EvalCondition(c->binop_l) || EvalCondition(c->binop_r);
	case Condition::SVAR: return GetSvar({c->svar.var}) == c->svar.val;
	}
	return false;
}

// Parsing {{{

enum TokenType {
	TOK_LPAREN,
	TOK_RPAREN,
	TOK_NOT,
	TOK_AND,
	TOK_OR,
	TOK_EQUALS,
	TOK_STR,
};

struct Token {
	enum TokenType type;
	// These are for TOK_STR
	const char *str;
	size_t len;
};

static bool _isIdentChar(char c) {
	return c != ' ' &&
		c != '\v' &&
		c != '\t' &&
		c != '\r' &&
		c != '\n' &&
		c != '(' &&
		c != ')' &&
		c != '!' &&
		c != '&' &&
		c != '|' &&
		c != '=';
}

static std::queue<Token> LexCondition(const char *str, size_t len) {
	std::queue<Token> toks;

	const char *end = str + len;

	while (str < end) {
		switch (*str) {
		case ' ':
		case '\v':
		case '\t':
		case '\r':
		case '\n':
			break;
		case '(':
			toks.push({TOK_LPAREN});
			break;
		case ')':
			toks.push({TOK_RPAREN});
			break;
		case '!':
			toks.push({TOK_NOT});
			break;
		case '&':
			toks.push({TOK_AND});
			break;
		case '|':
			toks.push({TOK_OR});
			break;
		case '=':
			toks.push({TOK_EQUALS});
			break;
		default:
			const char *start = str;
			while (str < end && _isIdentChar(*str)) {
				++str;
			}
			size_t len = str - start;
			toks.push({TOK_STR, start, len});
			continue;
		}
		++str;
	}

	return toks;
}

static Condition *ParseCondition(std::queue<Token> toks) {
	std::stack<enum TokenType> op_stack;
	std::stack<Condition *> out_stack;

#define CLEAR_OUT_STACK            \
	do {                              \
		while (!out_stack.empty()) {     \
			FreeCondition(out_stack.top()); \
			out_stack.pop();                \
		}                                \
	} while (0)

#define POP_OP_TO_OUTPUT                                        \
	do {                                                           \
		enum TokenType op = op_stack.top();                           \
		op_stack.pop();                                               \
		if (out_stack.empty()) {                                      \
			console->Print("Malformed input\n");                         \
			CLEAR_OUT_STACK;                                             \
			return NULL;                                                 \
		}                                                             \
		Condition *c_new = (Condition *)malloc(sizeof *c_new);        \
		if (op == TOK_NOT) {                                          \
			c_new->type = Condition::NOT;                                \
			c_new->unop_cond = out_stack.top();                          \
			out_stack.pop();                                             \
		} else {                                                      \
			c_new->type = op == TOK_OR ? Condition::OR : Condition::AND; \
			c_new->binop_r = out_stack.top();                            \
			out_stack.pop();                                             \
			if (out_stack.empty()) {                                     \
				console->Print("Malformed input\n");                        \
				CLEAR_OUT_STACK;                                            \
				return NULL;                                                \
			}                                                            \
			c_new->binop_l = out_stack.top();                            \
			out_stack.pop();                                             \
		}                                                             \
		out_stack.push(c_new);                                        \
	} while (0)

	while (!toks.empty()) {
		Token t = toks.front();
		toks.pop();

		switch (t.type) {
		// TOK_STR {{{
		case TOK_STR: {
			Condition *c = (Condition *)malloc(sizeof *c);

			if (!strncmp(t.str, "orange", t.len)) {
				c->type = Condition::ORANGE;
			} else if (t.len == 4 && !strncmp(t.str, "coop", t.len)) {
				c->type = Condition::COOP;
			} else if (t.len == 2 && !strncmp(t.str, "cm", t.len)) {
				c->type = Condition::CM;
			} else if (t.len == 8 && !strncmp(t.str, "same_map", t.len)) {
				c->type = Condition::SAME_MAP;
			} else if (t.len == 8 && !strncmp(t.str, "workshop", t.len)) {
				c->type = Condition::WORKSHOP;
			} else if (t.len == 4 && !strncmp(t.str, "menu", t.len)) {
				c->type = Condition::MENU;
			} else if (t.len == 3 && !strncmp(t.str, "map", t.len) || t.len == 8 && !strncmp(t.str, "prev_map", t.len) || t.len == 4 && !strncmp(t.str, "game", t.len) || t.len > 4 && !strncmp(t.str, "var:", 4)) {
				bool is_var = !strncmp(t.str, "var:", 4);

				if (toks.front().type != TOK_EQUALS) {
					console->Print("Expected = after '%*s'\n", t.len, t.str);
					CLEAR_OUT_STACK;
					return NULL;
				}

				toks.pop();

				Token map_tok = toks.front();
				toks.pop();

				if (map_tok.type != TOK_STR) {
					console->Print("Expected string token after '%*s='\n", t.len, t.str);
					CLEAR_OUT_STACK;
					return NULL;
				}

				if (is_var) {
					c->type = Condition::SVAR;
					c->svar.var = (char *)malloc(t.len - 4 + 1);
					strncpy(c->svar.var, t.str + 4, t.len - 4);
					c->svar.var[t.len - 4] = 0;  // Null terminator
					c->svar.val = (char *)malloc(map_tok.len + 1);
					strncpy(c->svar.val, map_tok.str, map_tok.len);
					c->svar.val[map_tok.len] = 0;  // Null terminator
				} else {
					c->type = t.len == 8 ? Condition::PREV_MAP : t.len == 4 ? Condition::GAME : Condition::MAP;
					c->map = (char *)malloc(map_tok.len + 1);
					strncpy(c->map, map_tok.str, map_tok.len);
					c->map[map_tok.len] = 0;  // Null terminator
				}
			} else {
				console->Print("Bad token '%.*s'\n", t.len, t.str);
				CLEAR_OUT_STACK;
				return NULL;
			}

			out_stack.push(c);

			break;
		}
		// }}}

		// TOK_LPAREN / TOK_NOT {{{
		case TOK_LPAREN:
		case TOK_NOT: {
			op_stack.push(t.type);
			break;
		}
			// }}}

		// TOK_RPAREN {{{
		case TOK_RPAREN:
			while (!op_stack.empty() && op_stack.top() != TOK_LPAREN) {
				POP_OP_TO_OUTPUT;
			}

			if (op_stack.empty()) {
				console->Print("Unmatched parentheses\n");
				CLEAR_OUT_STACK;
				return NULL;
			}

			op_stack.pop();

			break;
			// }}}

		// TOK_AND / TOK_OR {{{
		case TOK_AND:
		case TOK_OR: {
			while (!op_stack.empty() && (op_stack.top() == TOK_NOT || op_stack.top() == TOK_AND)) {
				POP_OP_TO_OUTPUT;
			}
			op_stack.push(t.type);
			break;
		}
		// }}}

		// TOK_EQUALS {{{
		case TOK_EQUALS: {
			console->Print("Unexpected '=' token\n");
			CLEAR_OUT_STACK;
			return NULL;
		}
			// }}}
		}
	}

	while (!op_stack.empty()) {
		POP_OP_TO_OUTPUT;
	}

#undef POP_OP_TO_OUTPUT
#undef CLEAR_OUT_STACK

	if (out_stack.empty()) {
		console->Print("Malformed input\n");
		return NULL;
	}

	Condition *cond = out_stack.top();
	out_stack.pop();

	if (!out_stack.empty()) {
		console->Print("Malformed input\n");
		FreeCondition(cond);
		return NULL;
	}

	return cond;
}

// }}}

#define MK_SAR_ON(name, when, immediately)                                                                                           \
	static std::vector<std::string> _g_execs_##name;                                                                                    \
	CON_COMMAND_F(sar_on_##name, "sar_on_" #name " <command> [args]... - registers a command to be run " when "\n", FCVAR_DONTRECORD) { \
		if (args.ArgC() < 2) {                                                                                                             \
			return console->Print(sar_on_##name.ThisPtr()->m_pszHelpString);                                                                  \
		}                                                                                                                                  \
		const char *cmd = args.ArgC() == 2 ? args[1] : args.m_pArgSBuffer + args.m_nArgv0Size;                                             \
		_g_execs_##name.push_back(std::string(cmd));                                                                                       \
	}                                                                                                                                   \
	static void _runExecs_##name() {                                                                                                    \
		for (auto cmd : _g_execs_##name) {                                                                                                 \
			engine->ExecuteCommand(cmd.c_str(), immediately);                                                                                 \
		}                                                                                                                                  \
	}

#define RUN_EXECS(x) _runExecs_##x()

MK_SAR_ON(load, "on session start", true)
MK_SAR_ON(session_end, "on session end", true)
MK_SAR_ON(exit, "on game exit", false)
MK_SAR_ON(demo_start, "when demo playback starts", false)
MK_SAR_ON(demo_stop, "when demo playback stops", false)
MK_SAR_ON(flags, "when CM flags are hit", false)
MK_SAR_ON(coop_reset_done, "when coop reset is completed", false)
MK_SAR_ON(coop_reset_remote, "when coop reset run remotely", false)

struct Seq {
	std::queue<std::string> commands;
};

std::vector<Seq> seqs;

CON_COMMAND_F(cond, "cond <condition> <command> [args]... - runs a command only if a given condition is met\n", FCVAR_DONTRECORD) {
	if (args.ArgC() < 3) {
		return console->Print(cond.ThisPtr()->m_pszHelpString);
	}

	const char *cond_str = args[1];

	Condition *cond = ParseCondition(LexCondition(cond_str, strlen(cond_str)));

	if (!cond) {
		console->Print("Condition parsing failed\n");
		return;
	}

	if (EvalCondition(cond)) {
		std::string cmd = args[2];
		for (int i = 3; i < args.ArgC(); ++i) {
			cmd += Utils::ssprintf(" \"%s\"", args[i]);
		}
		engine->ExecuteCommand(cmd.c_str());
	}

	FreeCondition(cond);
}

CON_COMMAND_F(seq, "seq <commands>... - runs a sequence of commands one tick after one another\n", FCVAR_DONTRECORD) {
	if (args.ArgC() < 2) {
		return console->Print(seq.ThisPtr()->m_pszHelpString);
	}

	int tick = session->GetTick();

	std::queue<std::string> cmds;
	for (int i = 1; i < args.ArgC(); ++i) {
		cmds.push(std::string(args[i]));
		if (engine->demorecorder->isRecordingDemo && *args[i]) {
			size_t size = strlen(args[i]) + 6;
			char *data = new char[size];
			data[0] = 0x09;
			*(int *)(data + 1) = tick + i;
			strcpy(data + 5, args[i]);
			engine->demorecorder->RecordData(data, size);
			delete[] data;
		}
	}

	seqs.push_back({cmds});
}

ON_EVENT(PRE_TICK) {
	for (size_t i = 0; i < seqs.size(); ++i) {
		if (seqs[i].commands.empty()) {
			seqs.erase(seqs.begin() + i);
			i--;  // Decrement the index to account for the removed element
			continue;
		}

		std::string cmd = seqs[i].commands.front();
		seqs[i].commands.pop();

		engine->ExecuteCommand(cmd.c_str(), true);
	}
}

struct AliasInfo {
	Command *cmd;
	std::string run;
	char *name;
};

static std::map<std::string, AliasInfo> g_aliases;

static void _aliasCallback(const CCommand &args) {
	engine->ExecuteCommand(Utils::ssprintf("sar_alias_run %s", args.m_pArgSBuffer).c_str(), true);
}

CON_COMMAND_F(sar_alias, "sar_alias <name> [command] [args]... - create an alias, similar to the 'alias' command but not requiring quoting. If no command is specified, prints the given alias\n", FCVAR_DONTRECORD) {
	if (args.ArgC() < 2) {
		return console->Print(sar_alias.ThisPtr()->m_pszHelpString);
	}

	if (args.ArgC() == 2) {
		auto alias = g_aliases.find({args[1]});
		if (alias == g_aliases.end()) {
			console->Print("Alias %s does not exist\n", args[1]);
		} else {
			console->Print("%s\n", alias->second.run.c_str());
		}
		return;
	}

	const char *cmd;

	if (args.ArgC() == 3) {
		cmd = args[2];
	} else {
		cmd = args.m_pArgSBuffer + args.m_nArgv0Size;

		while (isspace(*cmd)) ++cmd;

		if (*cmd == '"') {
			cmd += strlen(args[1]) + 2;
		} else {
			cmd += strlen(args[1]);
		}

		while (isspace(*cmd)) ++cmd;
	}

	auto existing = g_aliases.find({args[1]});
	if (existing != g_aliases.end()) {
		AliasInfo info = g_aliases[std::string(args[1])];
		info.cmd->Unregister();
		delete info.cmd;
		free(info.name);
	}

	if (Command(args[1]).ThisPtr()) {
		console->Print("Command %s already exists! Cannot shadow.\n", args[1]);
		return;
	}

	char *name = strdup(args[1]);  // We do this so that the ConCommand has a persistent handle to the command name
	Command *c = new Command(name, &_aliasCallback, "SAR alias command.\n", FCVAR_DONTRECORD);
	c->Register();
	g_aliases[std::string(args[1])] = {c, cmd, name};
}

CON_COMMAND_F(sar_alias_run, "sar_alias_run <name> [args]... - run a SAR alias, passing on any additional arguments\n", FCVAR_DONTRECORD) {
	if (args.ArgC() < 2) {
		return console->Print(sar_alias_run.ThisPtr()->m_pszHelpString);
	}

	auto it = g_aliases.find({args[1]});
	if (it == g_aliases.end()) {
		return console->Print("Alias %s does not exist\n", args[1]);
	}

	const char *argstr = args.m_pArgSBuffer + args.m_nArgv0Size;

	while (isspace(*argstr)) ++argstr;

	if (*argstr == '"') {
		argstr += strlen(args[1]) + 2;
	} else {
		argstr += strlen(args[1]);
	}

	while (isspace(*argstr)) ++argstr;

	std::string cmd = it->second.run + " " + argstr;

	engine->ExecuteCommand(cmd.c_str(), true);
}

ON_EVENT_P(SESSION_START, 1000000) {
	RUN_EXECS(load);
}
ON_EVENT(SESSION_END) {
	RUN_EXECS(session_end);
}
ON_EVENT(SAR_UNLOAD) {
	RUN_EXECS(exit);
}
ON_EVENT(DEMO_START) {
	RUN_EXECS(demo_start);
}
ON_EVENT(DEMO_STOP) {
	RUN_EXECS(demo_stop);
}
ON_EVENT(CM_FLAGS) {
	RUN_EXECS(flags);
}
ON_EVENT(COOP_RESET_DONE) {
	RUN_EXECS(coop_reset_done);
}
ON_EVENT(COOP_RESET_REMOTE) {
	RUN_EXECS(coop_reset_remote);
}

CON_COMMAND_F(nop, "nop [args]... - nop ignores all its arguments and does nothing\n", FCVAR_DONTRECORD) {}

static std::map<std::string, AliasInfo> g_functions;

static void _functionCallback(const CCommand &args) {
	engine->ExecuteCommand(Utils::ssprintf("sar_function_run %s", args.m_pArgSBuffer).c_str(), true);
}

CON_COMMAND_F(sar_function, "sar_function <name> [command] [args]... - create a function, replacing $1, $2 etc up to $9 in the command string with the respective argument. If no command is specified, prints the given function\n", FCVAR_DONTRECORD) {
	if (args.ArgC() < 2) {
		return console->Print(sar_function.ThisPtr()->m_pszHelpString);
	}

	if (args.ArgC() == 2) {
		auto func = g_functions.find({args[1]});
		if (func == g_functions.end()) {
			console->Print("Function %s does not exist\n", args[1]);
		} else {
			console->Print("%s\n", func->second.run.c_str());
		}
		return;
	}

	const char *cmd;

	if (args.ArgC() == 3) {
		cmd = args[2];
	} else {
		cmd = args.m_pArgSBuffer + args.m_nArgv0Size;

		while (isspace(*cmd)) ++cmd;

		if (*cmd == '"') {
			cmd += strlen(args[1]) + 2;
		} else {
			cmd += strlen(args[1]);
		}

		while (isspace(*cmd)) ++cmd;
	}

	auto existing = g_functions.find({args[1]});
	if (existing != g_functions.end()) {
		AliasInfo info = g_functions[std::string(args[1])];
		info.cmd->Unregister();
		delete info.cmd;
		free(info.name);
	}

	if (Command(args[1]).ThisPtr()) {
		console->Print("Command %s already exists! Cannot shadow.\n", args[1]);
		return;
	}

	char *name = strdup(args[1]);  // We do this so that the ConCommand has a persistent handle to the command name
	Command *c = new Command(name, &_functionCallback, "SAR function command.\n", FCVAR_DONTRECORD);
	c->Register();
	g_functions[std::string(args[1])] = {c, cmd, name};
}

static void expand(size_t nargs, const char *const *args, std::string body) {
	std::string cmd;

	for (size_t i = 0; i < body.size(); ++i) {
		if (body[i] == '$') {
			if (body[i + 1] == '$') {
				cmd += '$';
				++i;
				continue;
			} else if (body[i + 1] == '\'') {
				cmd += '"';
				++i;
				continue;
			} else if (body[i + 1] == '-') {
				// $- is just a no-op, so that you can have strings next
				// to svar subs
				++i;
				continue;
			} else if (body[i + 1] >= '1' && body[i + 1] <= '9') {
				int arg = body[i + 1] - '0';
				cmd += arg - 1 < nargs ? args[arg - 1] : "";
				++i;
				continue;
			} else {
				size_t len = 0;

				while (1) {
					char c = body[i + ++len];
					if (c >= 'a' && c <= 'z') continue;
					if (c >= 'A' && c <= 'Z') continue;
					if (c >= '0' && c <= '9') continue;
					if (c == '_' || c == '-') continue;
					break;
				}

				--len;

				if (len == 0) {
					cmd += '$';
					continue;
				}

				std::string var = body.substr(i + 1, len);
				cmd += GetSvar(var);
				i += len;

				continue;
			}
		}
		cmd += body[i];
	}

	engine->ExecuteCommand(cmd.c_str(), true);
}

CON_COMMAND_F(sar_expand, "sar_expand [cmd]... - run a command after expanding svar substitutions\n", FCVAR_DONTRECORD) {
	if (args.ArgC() < 2) {
		return console->Print(sar_expand.ThisPtr()->m_pszHelpString);
	}

	const char *cmd = args.ArgC() == 2 ? args[1] : args.m_pArgSBuffer + args.m_nArgv0Size;
	expand(0, nullptr, std::string(cmd));
}

CON_COMMAND_F(sar_function_run, "sar_function_run <name> [args]... - run a function with the given arguments\n", FCVAR_DONTRECORD) {
	if (args.ArgC() < 2) {
		return console->Print(sar_function_run.ThisPtr()->m_pszHelpString);
	}

	auto it = g_functions.find({args[1]});
	if (it == g_functions.end()) {
		return console->Print("Function %s does not exist\n", args[1]);
	}

	expand(args.ArgC() - 2, args.m_ppArgv + 2, it->second.run);
}
