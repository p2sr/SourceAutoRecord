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
#include <unordered_set>
#include <fstream>

// Fuck you Windows
#ifdef _WIN32
#	define strdup _strdup
#endif

#define PERSISTENT_SVAR_FILENAME "svars_persist"

static std::map<std::string, std::string> g_svars;
static std::unordered_set<std::string> g_persistentSvars;

ON_INIT {
	std::ifstream file(PERSISTENT_SVAR_FILENAME);

	std::string line;
	std::getline(file, line);

	while (file) {
		std::string oldline = line;
		std::getline(file, line);
		if (oldline == "") continue; // skip empty lines
		if (!file) break;

		g_svars[oldline] = line;

		// get the next line so we don't re-use the value line
		std::getline(file, line);
	}
}

static void SavePersistentSvars() {
	FILE *fp = fopen(PERSISTENT_SVAR_FILENAME, "w");
	if (fp) {
		for (auto &name : g_persistentSvars) {
			auto val = g_svars[name];
			fprintf(fp, "%s\n%s\n", name.c_str(), val.c_str());
		}
		fclose(fp);
	}
}

static void SetSvar(std::string name, std::string val) {
	g_svars[name] = val;
	if (g_persistentSvars.count(name) != 0) SavePersistentSvars();
}

static std::string GetSvar(std::string name) {
	auto it = g_svars.find(name);
	if (it == g_svars.end()) return "";
	return it->second;
}

static std::string GetCvar(std::string name) {
	Variable cvar(name.c_str());
	if (!cvar.ThisPtr() || cvar.ThisPtr()->IsCommand()) return "";
	return cvar.GetFlags() & FCVAR_NEVER_AS_STRING ? std::to_string(cvar.GetFloat()) : cvar.GetString();
}

static int CompleteSvars(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH], bool values) {
	auto args = ParsePartialArgs(partial);

	int completed_args = args.size() - 1;
	if (completed_args > (values ? 3 : 2)) completed_args = values ? 3 : 2;

	std::string part;
	for (int i = 0; i < completed_args; ++i) {
		if (args[i].find(" ") != std::string::npos) {
			part += "\"" + args[i] + "\" ";
		} else {
			part += args[i] + " ";
		}
	}

	if (completed_args == 0 || completed_args == (values ? 3 : 2)) {
		part = part.substr(0, part.size() - 1); // strip trailing space
		std::strncpy(commands[0], part.c_str(), COMMAND_COMPLETION_ITEM_LENGTH - 1);
		commands[0][COMMAND_COMPLETION_ITEM_LENGTH - 1] = 0;
		return 1;
	}

	if (values && completed_args == 2) {
		auto val = GetSvar(args[1]);
		if (val == "" || val.find(" ") != std::string::npos) val = "\"" + val + "\"";
		part += val;
		std::strncpy(commands[0], part.c_str(), COMMAND_COMPLETION_ITEM_LENGTH - 1);
		commands[0][COMMAND_COMPLETION_ITEM_LENGTH - 1] = 0;
		return 1;
	}

	std::string cur = args[args.size() - 1];

	std::vector<std::string> items;

	// completed_args == 1
	for (auto svar : g_svars) {
		if (Utils::StartsWith(svar.first.c_str(), "__")) continue;

		std::string qname =
			svar.first.find(" ") == std::string::npos
			? svar.first
			: Utils::ssprintf("\"%s\"", svar.first.c_str());

		if (svar.first == cur) {
			items.insert(items.begin(), part + qname);
		} else if (svar.first.find(cur) != std::string::npos) {
			items.push_back(part + qname);
		}

		if (items.size() >= COMMAND_COMPLETION_MAXITEMS) break;
	}

	for (size_t i = 0; i < items.size(); ++i) {
		std::strncpy(commands[i], items[i].c_str(), COMMAND_COMPLETION_ITEM_LENGTH - 1);
		commands[i][COMMAND_COMPLETION_ITEM_LENGTH - 1] = 0;
	}

	return items.size();
}

DECL_DECLARE_AUTOCOMPLETION_FUNCTION(svar_set) {
	return CompleteSvars(partial, commands, true);
}

DECL_DECLARE_AUTOCOMPLETION_FUNCTION(svar_get) {
	return CompleteSvars(partial, commands, false);
}

CON_COMMAND_F_COMPLETION(svar_set, "svar_set <variable> <value> - set a svar (SAR variable) to a given value\n", FCVAR_DONTRECORD, AUTOCOMPLETION_FUNCTION(svar_set)) {
	if (args.ArgC() < 3) {
		return console->Print(svar_set.ThisPtr()->m_pszHelpString);
	}

	const char *cmd = Utils::ArgContinuation(args, 2);

	SetSvar({args[1]}, {cmd});
}

CON_COMMAND_F_COMPLETION(svar_substr, "svar_substr <variable> <from> [len] - sets a svar to its substring.\n", FCVAR_DONTRECORD, AUTOCOMPLETION_FUNCTION(svar_get)) {
	if (args.ArgC() < 3 || args.ArgC() > 4) {
		return console->Print(svar_substr.ThisPtr()->m_pszHelpString);
	}

	std::string value = GetSvar({args[1]});
	int from = atoi(args[2]);
	int len = args.ArgC() == 4 ? atoi(args[3]) : value.length();

	if (from < 0) from += value.length();

	if (from < 0 || (unsigned)from > value.length()) {
		return console->Print("Substring index out of bounds of variable\n");
	}
	if (len < 0) {
		return console->Print("Negative length of substring\n");
	}

	value = value.substr(from, len);

	SetSvar({args[1]}, value);
}

CON_COMMAND_F_COMPLETION(svar_get, "svar_get <variable> - get the value of a svar\n", FCVAR_DONTRECORD, AUTOCOMPLETION_FUNCTION(svar_get)) {
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

	console->Print("%d svars defined\n", size);
}

CON_COMMAND_F_COMPLETION(svar_persist, "svar_persist <variable> - mark an svar as persistent\n", FCVAR_DONTRECORD, AUTOCOMPLETION_FUNCTION(svar_get)) {
	if (args.ArgC() != 2) {
		return console->Print(svar_persist.ThisPtr()->m_pszHelpString);
	}

	g_persistentSvars.insert({args[1]});
	SavePersistentSvars();
}

CON_COMMAND_F_COMPLETION(svar_no_persist, "svar_no_persist <variable> - unmark an svar as persistent\n", FCVAR_DONTRECORD, AUTOCOMPLETION_FUNCTION(svar_get)) {
	if (args.ArgC() != 2) {
		return console->Print(svar_no_persist.ThisPtr()->m_pszHelpString);
	}

	g_persistentSvars.erase({args[1]});
	SavePersistentSvars();
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

	SetSvar(g_svarListenerTarget, out);
}

CON_COMMAND_F_COMPLETION(svar_capture, "svar_capture <variable> <command> [args]... - capture a command's output and place it into an svar, removing newlines\n", FCVAR_DONTRECORD, AUTOCOMPLETION_FUNCTION(svar_get)) {
	if (args.ArgC() < 3) {
		return console->Print(svar_capture.ThisPtr()->m_pszHelpString);
	}

	const char *cmd = Utils::ArgContinuation(args, 2);

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

CON_COMMAND_F_COMPLETION(svar_from_cvar, "svar_from_cvar <variable> <cvar> - capture a cvar's value and place it into an svar, removing newlines\n", FCVAR_DONTRECORD, AUTOCOMPLETION_FUNCTION(svar_get)) {
	if (args.ArgC() != 3) {
		return console->Print(svar_from_cvar.ThisPtr()->m_pszHelpString);
	}

	Variable cvar(args[2]);

	if (cvar.ThisPtr()) {
		if (!cvar.ThisPtr()->IsCommand()) {
			std::string val = cvar.GetFlags() & FCVAR_NEVER_AS_STRING ? std::to_string(cvar.GetFloat()) : cvar.GetString();
			val.erase(std::remove(val.begin(), val.end(), '\n'), val.end());
			SetSvar({args[1]}, val);
		}
	}
}

#define SVAR_OP(name, op, disallowSecondZero)                      \
	CON_COMMAND_F_COMPLETION(svar_##name, "svar_" #name " <variable> <variable|value> - perform the given operation on an svar\n", FCVAR_DONTRECORD, AUTOCOMPLETION_FUNCTION(svar_get)) { \
		if (args.ArgC() != 3) {                                          \
			return console->Print(svar_##name.ThisPtr()->m_pszHelpString);  \
		}                                                                \
		int cur;                                                         \
		{                                                                \
			auto it = g_svars.find({args[1]});                              \
			cur = (it == g_svars.end()) ? 0 : atoi(it->second.c_str());     \
		}                                                                \
		char *end;                                                       \
		int other = strtol(args[2], &end, 10);                           \
		if (end == args[2] || *end) {                                    \
			auto it = g_svars.find({args[2]});                              \
			other = (it == g_svars.end()) ? 0 : atoi(it->second.c_str());   \
		}                                                                \
		int val = (disallowSecondZero && other == 0) ? 0 : op;           \
		SetSvar({args[1]}, Utils::ssprintf("%d", val));                  \
	}                                                                 \
	                                                                  \
	CON_COMMAND_F_COMPLETION(svar_f##name, "svar_f" #name " <variable> <variable|value> - perform the given operation on an svar\n", FCVAR_DONTRECORD, AUTOCOMPLETION_FUNCTION(svar_get)) { \
		if (args.ArgC() != 3) {                                          \
			return console->Print(svar_f##name.ThisPtr()->m_pszHelpString); \
		}                                                                \
		double cur;                                                      \
		{                                                                \
			auto it = g_svars.find({args[1]});                              \
			cur = (it == g_svars.end()) ? 0 : atof(it->second.c_str());     \
		}                                                                \
		char *end;                                                       \
		double other = strtod(args[2], &end);                            \
		if (end == args[2] || *end) {                                    \
			auto it = g_svars.find({args[2]});                              \
			other = (it == g_svars.end()) ? 0 : atof(it->second.c_str());   \
		}                                                                \
		double val = (disallowSecondZero && other == 0) ? 0 : op;        \
		SetSvar({args[1]}, Utils::ssprintf("%.17g", val));                  \
	}

SVAR_OP(add, cur + other, false)
SVAR_OP(sub, cur - other, false)
SVAR_OP(mul, cur * other, false)
SVAR_OP(div, cur / other, true)
SVAR_OP(mod, fmod(cur, other), true) // fmod seems to work fine for integers

#define SVAR_SINGLE_OP(name, op)\
	CON_COMMAND_F_COMPLETION(svar_##name, "svar_" #name " <variable> - perform the given operation on an svar\n", FCVAR_DONTRECORD, AUTOCOMPLETION_FUNCTION(svar_get)) { \
		if (args.ArgC() != 2) {                                            \
			return console->Print(svar_##name.ThisPtr()->m_pszHelpString);    \
		}                                                                  \
                                                                     \
		auto it = g_svars.find({args[1]});                                 \
		double cur = (it == g_svars.end()) ? 0 : atof(it->second.c_str()); \
                                                                     \
		SetSvar({args[1]}, Utils::ssprintf("%.17g", op));                     \
	}

SVAR_SINGLE_OP(round, round(cur))
SVAR_SINGLE_OP(floor, floor(cur))
SVAR_SINGLE_OP(ceil, ceil(cur))
SVAR_SINGLE_OP(abs, fabs(cur))

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
		CVAR,
		STRING,
		LINUX
	} type;

	union {
		struct {
			char *var, *val;
		};
		Condition *unop_cond;
		struct {
			Condition *binop_l, *binop_r;
		};
	};
};

static void FreeCondition(Condition *c) {
	switch (c->type) {
	case Condition::MAP:
	case Condition::PREV_MAP:
	case Condition::GAME:
		free(c->val);
		break;
	case Condition::SVAR:
	case Condition::CVAR:
	case Condition::STRING:
		free(c->var);
		free(c->val);
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

static const char *gameName() {
	switch (sar.game->GetVersion()) {
	case SourceGame_ApertureTag: return "aptag";
	case SourceGame_PortalStoriesMel: return "mel";
	case SourceGame_ThinkingWithTimeMachine: return "twtm";
	case SourceGame_PortalReloaded: return "reloaded";
	case SourceGame_Portal2: return Game::IsSpeedrunMod() ? "srm" : "portal2";
	default: return "other";
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
	case Condition::MAP: return !strcmp(c->val, engine->GetCurrentMapName().c_str());
	case Condition::PREV_MAP: return !strcmp(c->val, session->previousMap.c_str());
	case Condition::GAME: return !strcmp(c->val, gameName());
	case Condition::NOT: return !EvalCondition(c->unop_cond);
	case Condition::AND: return EvalCondition(c->binop_l) && EvalCondition(c->binop_r);
	case Condition::OR: return EvalCondition(c->binop_l) || EvalCondition(c->binop_r);
	case Condition::SVAR: return GetSvar({c->var}) == c->val;
	case Condition::CVAR: return GetCvar({c->var}) == c->val;
	case Condition::STRING: return !strcmp(c->var, c->val);
	case Condition::LINUX:
		#ifdef _WIN32
			return false;
		#else
			return true;
		#endif
	}
	return false;
}

// Condition Parsing {{{

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
				free(c_new);                                                \
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
			} else if (t.len == 5 && !strncmp(t.str, "linux", t.len)) {
				c->type = Condition::LINUX;
			} else if (
				t.len == 3 && !strncmp(t.str, "map", t.len) ||
				t.len == 8 && !strncmp(t.str, "prev_map", t.len) ||
				t.len == 4 && !strncmp(t.str, "game", t.len) ||
				t.len > 4 && !strncmp(t.str, "var:", 4) ||
				t.len > 5 && !strncmp(t.str, "cvar:", 5) ||
				t.len > 1 && (t.str[0] == '?' || t.str[0] == '#' || t.str[0] == '%')) {

				if (toks.empty() || toks.front().type != TOK_EQUALS) {
					console->Print("Expected = after '%.*s'\n", t.len, t.str);
					CLEAR_OUT_STACK;
					free(c);
					return NULL;
				}

				toks.pop();

				if (toks.empty() || toks.front().type != TOK_STR) {
					console->Print("Expected string token after '%.*s='\n", t.len, t.str);
					CLEAR_OUT_STACK;
					free(c);
					return NULL;
				}

				Token val_tok = toks.front();
				toks.pop();

				if (!strncmp(t.str, "var:", 4) || t.str[0] == '?') {
					int i = t.str[0] == 'v' ? 4 : 1;
					c->type = Condition::SVAR;
					c->var = (char *)malloc(t.len - i + 1);
					strncpy(c->var, t.str + i, t.len - i);
					c->var[t.len - i] = 0;  // Null terminator
				} else if (!strncmp(t.str, "cvar:", 5) || t.str[0] == '#') {
					int i = t.str[0] == 'c' ? 5 : 1;
					c->type = Condition::CVAR;
					c->var = (char *)malloc(t.len - i + 1);
					strncpy(c->var, t.str + i, t.len - i);
					c->var[t.len - i] = 0;  // Null terminator
				} else if (t.str[0] == '%') {
					c->type = Condition::STRING;
					c->var = (char *)malloc(t.len);
					strncpy(c->var, t.str + 1, t.len - 1);
					c->var[t.len - 1] = 0;  // Null terminator
				} else {
					c->type = t.len == 8 ? Condition::PREV_MAP : t.len == 4 ? Condition::GAME : Condition::MAP;
				}

				if (val_tok.len > 4 && !strncmp(val_tok.str, "var:", 4) || val_tok.len > 1 && val_tok.str[0] == '?') {
					int i = val_tok.str[0] == 'v' ? 4 : 1;
					auto val = GetSvar(std::string(val_tok.str + i, val_tok.len - i));
					c->val = strdup(val.c_str());
				} else if (val_tok.len > 5 && !strncmp(val_tok.str, "cvar:", 5) || val_tok.len > 1 && val_tok.str[0] == '#') {
					int i = val_tok.str[0] == 'c' ? 5 : 1;
					auto val = GetCvar(std::string(val_tok.str + i, val_tok.len - i));
					c->val = strdup(val.c_str());
				} else {
					c->val = (char *)malloc(val_tok.len + 1);
					strncpy(c->val, val_tok.str, val_tok.len);
					c->val[val_tok.len] = 0;  // Null terminator
				}
			} else {
				console->Print("Bad token '%.*s'\n", t.len, t.str);
				CLEAR_OUT_STACK;
				free(c);
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

CON_COMMAND_F(cond, "cond <condition> <command> [args]... - runs a command only if a given condition is met\n", FCVAR_DONTRECORD) {
	if (args.ArgC() < 3) {
		return console->Print(cond.ThisPtr()->m_pszHelpString);
	}

	const char *cond_str = args[1];

	Condition *cond = ParseCondition(LexCondition(cond_str, strlen(cond_str)));

	if (!cond) {
		console->Print("Condition parsing of \"%s\" failed\n", cond_str);
		return;
	}

	bool should_run = EvalCondition(cond);
	FreeCondition(cond);

	if (!should_run) return;

	const char *cmd = Utils::ArgContinuation(args, 2);

	engine->ExecuteCommand(cmd, true);
}

CON_COMMAND_F(conds, "conds [<condition> <command>]... [else] - runs the first command which has a satisfied condition\n", FCVAR_DONTRECORD) {
	if (args.ArgC() < 2) {
		return console->Print(conds.ThisPtr()->m_pszHelpString);
	}

	int i = 1;
	while (i < args.ArgC()) {
		if (i == (args.ArgC() - 1)) {
			// else
			engine->ExecuteCommand(args[i], true);
			return;
		}

		const char *cond_str = args[i];
		Condition *cond = ParseCondition(LexCondition(cond_str, strlen(cond_str)));
		if (!cond) {
			console->Print("Condition parsing of \"%s\" failed\n", cond_str);
			return;
		}

		bool should_run = EvalCondition(cond);
		FreeCondition(cond);

		if (should_run) {
			engine->ExecuteCommand(args[i + 1], true);
			return;
		}
		i += 2;
	}
}

#define MK_SAR_ON(name, when, immediately)                                                                                                \
	static std::vector<std::string> _g_execs_##name;                                                                                         \
	CON_COMMAND_F(sar_on_##name, "sar_on_" #name " <command> [args]... - registers a command to be run " when "\n", FCVAR_DONTRECORD) {      \
		if (args.ArgC() < 2) {                                                                                                                  \
			return console->Print(sar_on_##name.ThisPtr()->m_pszHelpString);                                                                       \
		}                                                                                                                                       \
		const char *cmd = Utils::ArgContinuation(args, 1);                                                  \
		_g_execs_##name.push_back(std::string(cmd));                                                                                            \
	}                                                                                                                                        \
	CON_COMMAND_F(sar_on_##name##_clear, "sar_on_" #name "_clear - clears commands registered on event \"" #name "\"\n", FCVAR_DONTRECORD) { \
		console->Print("Cleared %d commands from event \"" #name "\"\n", _g_execs_##name.size());                                               \
		_g_execs_##name.clear();                                                                                                                \
	}                                                                                                                                        \
	CON_COMMAND_F(sar_on_##name##_list, "sar_on_" #name "_list - lists commands registered on event \"" #name "\"\n", FCVAR_DONTRECORD) {     \
		console->Print("%d commands on event \"" #name "\"\n", _g_execs_##name.size());                                                         \
		for (auto cmd : _g_execs_##name) {                                                                                                      \
			console->Print("%s\n", cmd.c_str());                                                                                                   \
		}                                                                                                                                       \
	}                                                                                                                                        \
	static void _runExecs_##name() {                                                                                                         \
		for (auto cmd : _g_execs_##name) {                                                                                                      \
			engine->ExecuteCommand(cmd.c_str(), immediately);                                                                                      \
		}                                                                                                                                       \
	}

#define RUN_EXECS(x) _runExecs_##x()

MK_SAR_ON(load, "on session start", true)
MK_SAR_ON(session_end, "on session end", true)
MK_SAR_ON(exit, "on game exit", true)
MK_SAR_ON(demo_start, "when demo playback starts", false)
MK_SAR_ON(demo_stop, "when demo playback stops", false)
MK_SAR_ON(flags, "when CM flags are hit", false)
MK_SAR_ON(coop_reset_done, "when coop reset is completed", false)
MK_SAR_ON(coop_reset_remote, "when coop reset run remotely", false)
MK_SAR_ON(coop_spawn, "on coop spawn", true)
MK_SAR_ON(config_exec, "on config.cfg exec", true)
MK_SAR_ON(tas_start, "when TAS script playback starts", true)
MK_SAR_ON(tas_end, "when TAS script playback ends", true)
MK_SAR_ON(pb, "when auto-submitter detects PB", true)
MK_SAR_ON(not_pb, "when auto-submitter detects not PB", true)

ON_EVENT_P(SESSION_START, 1000000) {
	RUN_EXECS(load);
	if (engine->IsOrange()) RUN_EXECS(coop_spawn);
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
	if (event.end) RUN_EXECS(flags);
}
ON_EVENT(COOP_RESET_DONE) {
	RUN_EXECS(coop_reset_done);
}
ON_EVENT(COOP_RESET_REMOTE) {
	RUN_EXECS(coop_reset_remote);
}
ON_EVENT(ORANGE_READY) {
	RUN_EXECS(coop_spawn);
}
ON_EVENT(CONFIG_EXEC) {
	RUN_EXECS(config_exec);
}
ON_EVENT(TAS_START) {
	RUN_EXECS(tas_start);
}
ON_EVENT(TAS_END) {
	RUN_EXECS(tas_end);
}
ON_EVENT(MAYBE_AUTOSUBMIT) {
	if (event.pb) RUN_EXECS(pb);
	else RUN_EXECS(not_pb);
}

struct Seq {
	std::queue<std::string> commands;
};

std::vector<Seq> seqs;

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

	const char *cmd = Utils::ArgContinuation(args, 2);

	auto existing = g_aliases.find({args[1]});
	if (existing == g_aliases.end()) {
		if (Command(args[1]).ThisPtr()) {
			console->Print("Command %s already exists! Cannot shadow.\n", args[1]);
			return;
		} else {
			char *name = strdup(args[1]);  // We do this so that the ConCommand has a persistent handle to the command name
			Command *c = new Command(name, &_aliasCallback, "SAR alias command.\n", FCVAR_DONTRECORD);
			c->Register();
			g_aliases[std::string(args[1])] = {c, cmd, name};
		}
	} else {
		// redirect command exists already. just update it
		g_aliases[std::string(args[1])].run = cmd;
	}
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
	argstr += (*argstr == '"') * 2 + strlen(args[1]);
	while (isspace(*argstr)) ++argstr;

	std::string cmd = it->second.run + " " + argstr;

	engine->ExecuteCommand(cmd.c_str(), true);
}

static std::map<std::string, AliasInfo> g_functions;

static void _functionCallback(const CCommand &args) {
	engine->ExecuteCommand(Utils::ssprintf("sar_function_run %s", args.m_pArgSBuffer).c_str(), true);
}

CON_COMMAND_F(sar_function, "sar_function <name> [command] [args]... - create a function, replacing $1, $2 etc in the command string with the respective argument, and more. If no command is specified, prints the given function\n", FCVAR_DONTRECORD) {
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

	const char *cmd = Utils::ArgContinuation(args, 2);

	auto existing = g_functions.find({args[1]});
	if (existing == g_functions.end()) {
		if (Command(args[1]).ThisPtr()) {
			console->Print("Command %s already exists! Cannot shadow.\n", args[1]);
			return;
		} else {
			char *name = strdup(args[1]);  // We do this so that the ConCommand has a persistent handle to the command name
			Command *c = new Command(name, &_functionCallback, "SAR function command.\n", FCVAR_DONTRECORD);
			c->Register();
			g_functions[std::string(args[1])] = {c, cmd, name};
		}
	} else {
		// redirect command exists already. just update it
		g_functions[std::string(args[1])].run = cmd;
	}

}

static void expand(const CCommand &args, std::string body) {
	std::string cmd;
	int nargs = args.ArgC() - 2;
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
			} else if (body[i + 1] == '+') {
				++i;
				if (body[i + 1] >= '0' && body[i + 1] <= '9') {
					int arg = body[i + 1] - '0';
					++i;
					while (body[i + 1] >= '0' && body[i + 1] <= '9') {
						arg = arg * 10 + (body[i + 1] - '0');
						++i;
					}
					if (arg - 1 < nargs) {
						// Skip the first n + 1 arguments
						// (sar_function_run)
						const char *greedy = args.m_pArgSBuffer + args.m_nArgv0Size;
						while (isspace(*greedy)) ++greedy;
						for (int j = 1; j < arg + 1; ++j) {
							greedy += (*greedy == '"') * 2 + strlen(args[j]);
							while (isspace(*greedy)) ++greedy;
						}
						cmd += greedy;
					}
				} else {
					cmd += "$+";
				}
				continue;
			} else if (body[i + 1] == '#') {
				cmd += std::to_string(nargs);
				++i;
				continue;
			} else if (body[i + 1] >= '0' && body[i + 1] <= '9') {
				int arg = body[i + 1] - '0';
				++i;
				while (body[i + 1] >= '0' && body[i + 1] <= '9') {
					arg = arg * 10 + (body[i + 1] - '0');
					++i;
				}
				cmd += arg - 1 < nargs ? args[arg + 1] : "";
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

	const char *cmd = Utils::ArgContinuation(args, 1);
	const CCommand noArgs = {1};  // ArgC = 1 means nargs = -1
	expand(noArgs, std::string(cmd));
}

CON_COMMAND_F(sar_function_run, "sar_function_run <name> [args]... - run a function with the given arguments\n", FCVAR_DONTRECORD) {
	if (args.ArgC() < 2) {
		return console->Print(sar_function_run.ThisPtr()->m_pszHelpString);
	}

	auto it = g_functions.find({args[1]});
	if (it == g_functions.end()) {
		return console->Print("Function %s does not exist\n", args[1]);
	}

	expand(args, it->second.run);
}

CON_COMMAND_F(nop, "nop [args]... - nop ignores all its arguments and does nothing\n", FCVAR_DONTRECORD) {}
