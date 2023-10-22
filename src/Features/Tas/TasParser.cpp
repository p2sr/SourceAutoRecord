#include "TasParser.hpp"
#include "Modules/Console.hpp"
#include "TasPlayer.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <optional>
#include <cfloat>

struct TasToken {
	enum {
		PLUS,
		RIGHT_ANGLE,
		DOUBLE_RIGHT_ANGLE,
		PIPE,
		SEMICOLON,
		INTEGER,
		FLOAT,
		STRING,
	} type;

	std::string tok;

	union {
		int i;
		float f;
	};
};

struct Line {
	std::vector<TasToken> tokens;
	unsigned num;
};

static std::vector<Line> tokenize(std::istream &file) {
	std::vector<Line> lines;

	unsigned line_num = 0;
	bool commentOpen = false;
	std::string line;
	while (std::getline(file, line)) {
		line_num += 1;

		// FIXME: This doesn't work with nested comments e.g.: /* ... /* ... */ */
		auto multilineCommentStart = line.find("/*");
		auto multilineCommentEnd = line.find("*/");

		bool didOpenComment = false;

		if (multilineCommentStart != std::string::npos) {
			if (multilineCommentEnd != std::string::npos)
				line = line.erase(multilineCommentStart, (multilineCommentEnd - multilineCommentStart) + 2);
			else {
				line = line.substr(0, multilineCommentStart);
				commentOpen = true;
				didOpenComment = true;
			}
		}

		if (commentOpen && !didOpenComment) {
			if (multilineCommentEnd != std::string::npos) {
				line = line.substr(multilineCommentEnd + 2, line.length());
				commentOpen = false;
			} else {
				continue;
			}
		}

		if (line.empty()) continue;

		std::vector<TasToken> toks;

		int fieldnum = 0;

		for (size_t idx = 0; idx < line.size(); ++idx) {
			// Skip leading whitespace
			while (idx < line.size() && isspace(line[idx])) ++idx;
			if (idx == line.size()) break;

			if (idx < line.size() - 1 && line[idx] == '/' && line[idx+1] == '/') {
				// Comment - finish the line
				break;
			}

			if (fieldnum == 0 && line[idx] == '+') {
				toks.push_back({ TasToken::PLUS, "+" });
				continue;
			}

			if (fieldnum == 0 && line[idx] == '>') {
				// special case - tools-only bulk
				if (line.size() > idx + 1 && line[idx+1] == '>') {
					fieldnum = 5;
					idx++;
					toks.push_back({TasToken::DOUBLE_RIGHT_ANGLE, ">>"});
				} else {
					// regular case - normal framebulk
					++fieldnum;
					toks.push_back({TasToken::RIGHT_ANGLE, ">"});
				}
				continue;
			}

			if (line[idx] == '|') {
				++fieldnum;
				toks.push_back({ TasToken::PIPE, "|" });
				continue;
			}

			if (line[idx] == ';') {
				toks.push_back({ TasToken::SEMICOLON, ";" });
				continue;
			}

			// These are quite ugly special cases, but this grammar doesn't
			// nicely tokenize, so deal with it
			if (fieldnum == 4) {
				// Commands should be parsed as one big string
				// We'll parse the whole thing right now
				bool quoted = false;
				std::string cmd;
				while (idx < line.size() && (line[idx] != '|' || quoted)) {
					if (idx < line.size() - 1 && line[idx] == '/' && line[idx+1] == '/' && !quoted) break;
					cmd += line[idx];
					if (line[idx] == '"') quoted = !quoted;
					++idx;
				}
				toks.push_back({ TasToken::STRING, cmd });
				--idx;
			} else {
				// Do normal string parsing
				std::string tok;
				while (idx < line.size() && !isspace(line[idx])) {
					char c = line[idx];

					if (fieldnum == 0 && c == '+') break;
					if (fieldnum == 0 && c == '>') break;
					if (c == '|' || c == ';') break;
					if (idx < line.size() - 1 && line[idx] == '/' && line[idx+1] == '/') break;

					if (fieldnum == 3 && tok.size() > 0) {
						// Buttons should parse each alphabetical character as
						// separate strings. Gross, but fuck it
						char fst = tok[0];
						bool num = c >= '0' && c <= '9';
						bool first_num = fst >= '0' && fst <= '9';
						if (!num || !first_num) {
							// Don't parse this token! Stop here, we'll get it in
							// the next pass
							break;
						}
					}
					tok += c;
					++idx;
				}

				// Revert idx since it'll be incremented again after the loop
				--idx;

				if (tok.size() > 0) {
					TasToken t{ TasToken::STRING, tok };

					// Can we parse it as an int or float?
					const char *c_tok = tok.c_str();
					char *end;
					long l = strtol(c_tok, &end, 10);
					if (!*end) {
						// It was an integer!
						t.type = TasToken::INTEGER;
						t.i = l;
					} else {
						// Let's try float instead
						float f = strtof(c_tok, &end);
						if (!*end) {
							// It was a float!
							t.type = TasToken::FLOAT;
							t.f = f;
						}
					}

					toks.push_back(t);
				}
			}
		}

		if (toks.size() > 0) {
			lines.push_back(Line{ std::move(toks), line_num });
		}
	}

	return lines;
}

#define _STR1(x) #x
#define _STR(x) _STR1(x)

static int parseVersion(const Line &l) {
	if (l.tokens[0].tok != "version") {
		throw TasParserException("expected version line");
	}

	if (l.tokens.size() != 2) {
		throw TasParserException("invalid version line; expected 2 tokens");
	}

	if (l.tokens[1].type != TasToken::INTEGER) {
		throw TasParserException("invalid version line; expected integer version");
	}

	if (l.tokens[1].i < 1 || l.tokens[1].i > MAX_SCRIPT_VERSION) {
		throw TasParserException("expected version number between 1 and " _STR(MAX_SCRIPT_VERSION));
	}

	return l.tokens[1].i;
}

static bool parseRngManip(const Line &l, std::string &rngManip) {
	if (l.tokens[0].tok != "rngmanip") return false;

	if (l.tokens.size() != 2) {
		throw TasParserException("invalid rngmanip line; expected 2 tokens");
	}

	rngManip = l.tokens[1].tok;

	return true;
}

static TasScriptStartInfo parseStartInfo(const Line &l) {
	TasScriptStartInfo info;

	if (l.tokens[0].tok != "start") {
		throw TasParserException("expected start line");
	}

	if (l.tokens.size() < 2) {
		throw TasParserException("invalid start line; expected at least 2 tokens");
	}

#define CHECK_TOKS(n) \
	if (l.tokens.size() - idx != (n)) { \
		throw TasParserException( \
			Utils::ssprintf("invalid start line; expected %d tokens, got %d\n", (int)(n), (int)l.tokens.size()) \
		); \
	}

	int idx = 1;

	info.isNext = false;

	if (l.tokens[idx].tok == "next") {
		// next is followed by another start type to be triggered on the
		// session following another start
		info.isNext = true;
		if (l.tokens.size() == 2) {
			// 'start next' is an alias for 'start next now'
			info.type = TasScriptStartType::StartImmediately;
			info.param = "";
			return info;
		}
		idx += 1;
	}

	if (l.tokens[idx].tok == "map") {
		CHECK_TOKS(2)
		info.type = TasScriptStartType::ChangeLevel;
		info.param = l.tokens[idx + 1].tok;
	} else if (l.tokens[idx].tok == "save") {
		CHECK_TOKS(2)
		info.type = TasScriptStartType::LoadQuicksave;
		info.param = l.tokens[idx + 1].tok;
	} else if (l.tokens[idx].tok == "cm") {
		CHECK_TOKS(2)
		info.type = TasScriptStartType::ChangeLevelCM;
		info.param = l.tokens[idx + 1].tok;
	} else if (l.tokens[idx].tok == "now") {
		CHECK_TOKS(1)
		info.type = TasScriptStartType::StartImmediately;
		info.param = "";
	} else {
		throw TasParserException(Utils::ssprintf("invalid start type '%s'", l.tokens[idx].tok.c_str()));
	}

#undef CHECK_TOKS
	return info;
}

struct LoopInfo {
	std::vector<Line> before;
	std::shared_ptr<LoopInfo> parent;
	unsigned line_num;
	unsigned count;
	LoopInfo(std::vector<Line> before, std::shared_ptr<LoopInfo> parent, unsigned line_num, unsigned count)
		: before(before), parent(parent), line_num(line_num), count(count)
	{ }
};

static int parseFramebulkTick(int last_tick, const Line &line, size_t *tokens_out, bool *is_tool_bulk) {
	if (line.tokens[0].type == TasToken::PLUS) {
		if (line.tokens.size() < 3) {
			throw TasParserException("expected at least 3 tokens");
		}

		if (line.tokens[1].type != TasToken::INTEGER) {
			throw TasParserException("expected integer after +");
		}

		if (line.tokens[2].type != TasToken::RIGHT_ANGLE && line.tokens[2].type != TasToken::DOUBLE_RIGHT_ANGLE) {
			throw TasParserException("expected > or >> after tick");
		}

		if (line.tokens[1].i < 1) {
			throw TasParserException("expected positive tick delta");
		}

		if (last_tick == -1) {
			throw TasParserException("first framebulk in file is relative");
		}
		
		if (tokens_out) *tokens_out = 3;
		if (is_tool_bulk) *is_tool_bulk = (line.tokens[2].type == TasToken::DOUBLE_RIGHT_ANGLE);

		return last_tick + line.tokens[1].i;
	} else if (line.tokens[0].type == TasToken::INTEGER) {
		if (line.tokens.size() < 2) {
			throw TasParserException("expected at least 2 tokens");
		}

		if (line.tokens[1].type != TasToken::RIGHT_ANGLE && line.tokens[1].type != TasToken::DOUBLE_RIGHT_ANGLE) {
			throw TasParserException("expected > or >> after tick");
		}

		if (line.tokens[0].i < 0) {
			throw TasParserException("expected non-negative tick");
		}
		
		if (line.tokens[0].i <= last_tick) {
			throw TasParserException(Utils::ssprintf("expected tick > %d", last_tick));
		}

		if (tokens_out) *tokens_out = 2;
		if (is_tool_bulk) *is_tool_bulk = (line.tokens[1].type == TasToken::DOUBLE_RIGHT_ANGLE);

		return line.tokens[0].i;
	}

	throw TasParserException("expected tick at start of line");
}

static std::optional<TasToolCommand> parseToolCmd(const std::vector<std::string> &toks) {
	if (toks.size() == 0) return {};

	for (auto &tool : TasTool::GetList(0)) {
		if (tool->GetName() == toks[0]) {
			std::vector<std::string> args = std::vector(toks.begin() + 1, toks.end());

			auto params = tool->ParseParams(args);

			return TasToolCommand{tool, params};
		}
	}

	throw TasParserException(Utils::ssprintf("unknown tool %s", toks[0].c_str()));
}

static Vector parseVector(const Line &line, size_t idx) {
	if (idx >= line.tokens.size() - 1) {
		throw TasParserException("unexpected eol; expected vector");
	}

	auto &t0 = line.tokens[idx];
	auto &t1 = line.tokens[idx + 1];

	Vector vec;

	if (t0.type == TasToken::INTEGER) vec.x = t0.i;
	else if (t0.type == TasToken::FLOAT) vec.x = t0.f;
	else throw TasParserException("expected vector");

	if (t1.type == TasToken::INTEGER) vec.y = t1.i;
	else if (t1.type == TasToken::FLOAT) vec.y = t1.f;
	else throw TasParserException(Utils::ssprintf("expected vector B %d '%s'", (int)t1.type, t1.tok.c_str()));

	if (idx + 2 < line.tokens.size()) {
		auto &t3 = line.tokens[idx + 2];
		switch (t3.type) {
		case TasToken::PIPE:
			break;
		case TasToken::INTEGER:
		case TasToken::FLOAT:
			throw TasParserException("invalid vector param count");
		default:
			throw TasParserException(Utils::ssprintf("expected end of vector, found '%s'", t3.tok.c_str()));
		}
	}

	return vec;
}

static TasFramebulk parseFramebulk(int last_tick, const TasFramebulk &base, const Line &line, int (*button_timeouts)[TAS_CONTROLLER_INPUT_COUNT]) {
	TasFramebulk bulk = base;
	size_t toks_off;
	bool is_tool_bulk;
	bulk.tick = parseFramebulkTick(last_tick, line, &toks_off, &is_tool_bulk);

	int component = is_tool_bulk ? 4 : 0;

	for (size_t i = toks_off; i < line.tokens.size(); ++i) {
		if (line.tokens[i].type == TasToken::PIPE) {
			++component;
			if (component > 4) {
				throw TasParserException("unexpected |");
			}
			continue;
		}

		switch (component) {
		case 0: // Movement
			bulk.moveAnalog = parseVector(line, i);
			++i;
			break;
		case 1: // View
			bulk.viewAnalog = parseVector(line, i);
			++i;
			break;
		case 2: // Buttons
			{
				auto &t = line.tokens[i];
				if (t.type != TasToken::STRING) {
					throw TasParserException("expected string token");
				}

				if (t.tok.size() != 1) {
					throw TasParserException("button token too long !!! THIS IS A PARSER BUG !!!");
				}

				char c = t.tok[0];
				bool state = c >= 'A' && c <= 'Z';
				if (state) c += 'a' - 'A';

				TasControllerInput btn;

				switch (c) {
				case 'j':
					btn = TasControllerInput::Jump;
					break;
				case 'd':
					btn = TasControllerInput::Crouch;
					break;
				case 'u':
					btn = TasControllerInput::Use;
					break;
				case 'z':
					btn = TasControllerInput::Zoom;
					break;
				case 'b':
					btn = TasControllerInput::FireBlue;
					break;
				case 'o':
					btn = TasControllerInput::FireOrange;
					break;
				default:
					throw TasParserException("invalid button character");
				}

				if (i < line.tokens.size() - 1 && line.tokens[i + 1].type == TasToken::INTEGER) {
					int n = line.tokens[i + 1].i;
					if (state) {
						if (n <= 0) {
							throw TasParserException("button pressed for less than one tick");
						}
						(*button_timeouts)[btn] = bulk.tick + n;
						++i;
					} else {
						char c1 = c + 'A' - 'a';
						throw TasParserException(Utils::ssprintf("cannot release input with duration - did you mean %c%d?", c1, n));
					}
				}

				bulk.buttonStates[btn] = state;
			}
			break;
		case 3: // Commands
			if (line.tokens[i].type != TasToken::STRING) {
				throw TasParserException("unexpected token in command field");
			}
			bulk.commands.push_back(line.tokens[i].tok);
			break;
		case 4: // Tools
			{
				std::vector<std::string> args;
				while (i < line.tokens.size() && line.tokens[i].type >= TasToken::INTEGER) {
					args.push_back(line.tokens[i].tok);
					++i;
				}

				if (i < line.tokens.size()) {
					if (line.tokens[i].type == TasToken::PIPE) {
						--i; // Prevent double-increment
					} else if (line.tokens[i].type != TasToken::SEMICOLON) {
						throw TasParserException("unexpected token in tools field");
					}
				}

				auto cmd = parseToolCmd(args);
				if (cmd) bulk.toolCmds.push_back(*cmd);
			}
			break;
		}
	}

	return bulk;
}

static std::vector<TasFramebulk> parseFramebulks(const char *filepath, const Line *lines, size_t nlines) {
	int last_tick = -1;
	TasFramebulk last;
	std::vector<TasFramebulk> bulks;

	int button_timeouts[TAS_CONTROLLER_INPUT_COUNT];
	for (size_t i = 0; i < TAS_CONTROLLER_INPUT_COUNT; ++i) {
		button_timeouts[i] = -1;
	}

	for (size_t i = 0; i < nlines; ++i) {
		const Line &line = lines[i];
		try {
			auto fb_tick = parseFramebulkTick(last_tick, line, nullptr, nullptr);

			// Disable any buttons whose timeouts have expired
			for (int tick = last_tick + 1; tick < fb_tick; ++tick) {
				TasFramebulk mid_bulk = last;
				mid_bulk.tick = tick;
				mid_bulk.commands.clear();
				mid_bulk.toolCmds.clear();
				bool dirty = false;

				for (size_t i = 0; i < TAS_CONTROLLER_INPUT_COUNT; ++i) {
					if (button_timeouts[i] == tick) {
						dirty = true;
						mid_bulk.buttonStates[i] = false;
						button_timeouts[i] = -1;
					}
				}

				if (dirty) {
					bulks.push_back(mid_bulk);
					last = mid_bulk;
				}
			}

			TasFramebulk base = last;
			// Don't preserve commands or tool commands
			base.commands.clear();
			base.toolCmds.clear();
			// If any button timeouts are expiring this tick, do that in the
			// base
			for (size_t i = 0; i < TAS_CONTROLLER_INPUT_COUNT; ++i) {
				if (button_timeouts[i] == fb_tick) {
					base.buttonStates[i] = false;
					button_timeouts[i] = -1;
				}
			}

			TasFramebulk bulk = parseFramebulk(last_tick, base, line, &button_timeouts);

			bulks.push_back(bulk);
			last_tick = bulk.tick;
			last = bulk;
		} catch (TasParserException &e) {
			throw TasParserException(Utils::ssprintf("[%s:%u] %s", filepath, line.num, e.msg.c_str()));
		}
	}

	for (int tick = last_tick + 1;; ++tick) {
		TasFramebulk mid_bulk = last;
		mid_bulk.commands.clear();
		mid_bulk.toolCmds.clear();
		mid_bulk.tick = tick;
		bool dirty = false;
		bool done = true;

		for (size_t i = 0; i < TAS_CONTROLLER_INPUT_COUNT; ++i) {
			if (button_timeouts[i] != -1) done = false;

			if (button_timeouts[i] == tick) {
				dirty = true;
				mid_bulk.buttonStates[i] = false;
				button_timeouts[i] = -1;
			}
		}

		if (dirty) {
			bulks.push_back(mid_bulk);
			last = mid_bulk;
		} else if (done) {
			break;
		}
	}

	return bulks;
}

static std::vector<Line> preProcess(const char *filepath, const Line *lines, size_t nlines) {
	std::vector<Line> current;
	std::shared_ptr<LoopInfo> loop;

	for (size_t i = 0; i < nlines; ++i) {
		const Line &line = lines[i];
		try {
			if (line.tokens[0].tok == "repeat") {
				if (line.tokens.size() != 2) {
					throw TasParserException("invalid repeat line; expected 2 tokens");
				}

				if (line.tokens[1].type != TasToken::INTEGER) {
					throw TasParserException("invalid repeat line; expected integer");
				}

				if (line.tokens[1].i < 0) {
					throw TasParserException("invalid repeat line; expected integer >= 0");
				}

				loop = std::make_shared<LoopInfo>(
					current,
					loop,
					line.num,
					line.tokens[1].i
				);

				current = {};
			} else if (line.tokens[0].tok == "end") {
				if (line.tokens.size() != 1) {
					throw TasParserException("invalid end line; unexpected token");
				}

				if (!loop) {
					throw TasParserException("end line outside of a loop");
				}

				auto loop_contents = current;
				auto count = loop->count;
				current = loop->before;
				loop = loop->parent;

				// slow
				for (size_t i = 0; i < count; ++i) {
					for (auto &fb : loop_contents) {
						current.push_back(fb);
					}
				}
			} else {
				current.push_back(line);
			}
		} catch (TasParserException &e) {
			throw TasParserException(Utils::ssprintf("[%s:%u] %s", filepath, line.num, e.msg.c_str()));
		}
	}

	if (loop) {
		auto err = Utils::ssprintf("unterminated loop on line %d", loop->line_num);
		while ((loop = loop->parent)) {
			err += Utils::ssprintf("; in an unterminated loop on line %d", loop->line_num);
		}
		throw TasParserException(err);
	}

	return current;
}

static TasScript parseStream(std::string name, std::istream &stream) {
	TasScript script;
	
	auto lines = tokenize(stream);

	lines = preProcess(name.c_str(), lines.data(), lines.size());

	if (lines.size() < 2) {
		throw TasParserException(Utils::ssprintf("[%s] no lines in TAS script", name.c_str()));
	}

	try {
		script.header.version = parseVersion(lines[0]);
	} catch (TasParserException &e) {
		throw TasParserException(Utils::ssprintf("[%s:%u] %s", name.c_str(), lines[0].num, e.msg.c_str()));
	}

	try {
		script.header.startInfo = parseStartInfo(lines[1]);
	} catch (TasParserException &e) {
		throw TasParserException(Utils::ssprintf("[%s:%u] %s", name.c_str(), lines[1].num, e.msg.c_str()));
	}

	size_t script_start = 2;
	try {
		if (lines.size() > 2 && parseRngManip(lines[2], script.header.rngManipFile)) {
			script_start += 1;
		}
	} catch (TasParserException &e) {
		throw TasParserException(Utils::ssprintf("[%s:%u] %s", name.c_str(), lines[2].num, e.msg.c_str()));
	}

	script.framebulks = parseFramebulks(name.c_str(), lines.data() + script_start, lines.size() - script_start);  // skip version and start lines

	if (script.framebulks.size() == 0) {
		throw TasParserException(Utils::ssprintf("[%s] no framebulks in TAS script", name.c_str()));
	}

	return script;
}

TasScript TasParser::ParseFile(std::string filePath) {
	std::ifstream file(filePath, std::fstream::in);
	if (!file) {
		throw TasParserException(Utils::ssprintf("[%s] failed to open the file", filePath.c_str()));
	}

	auto result = parseStream(filePath, file);
	result.loadedFromFile = true;
	result.path = filePath;
	return result;
}

TasScript TasParser::ParseScript(std::string scriptName, std::string scriptString) {
	std::istringstream script(scriptString);

	auto result = parseStream(scriptName, script);
	result.loadedFromFile = false;
	return result;
}

int TasParser::toInt(std::string &str) {
	char *pEnd;
	const char *number = str.c_str();
	int x = std::strtol(number, &pEnd, 10);
	if (number == pEnd) {  //If no conversion
		throw TasParserException(str + " is not a number");
	}

	return x;
}

float TasParser::toFloat(std::string str) {
	char *pEnd;
	const char *number = str.c_str();
	float x = std::strtof(number, &pEnd);
	if (number == pEnd) {  //If no conversion
		throw TasParserException(str + " is not a number");
	}

	return x;
}

void TasParser::SaveRawScriptToFile(TasScript script) {
	std::string fixedName = script.path;
	size_t lastdot = fixedName.find_last_of(".");
	if (lastdot != std::string::npos) {
		fixedName = fixedName.substr(0, lastdot);
	}

	std::string fullRawPath = fixedName + "_raw." + TAS_SCRIPT_EXT;
	std::ofstream file(fullRawPath);

	file << SaveRawScriptToString(script);
	console->Print("Saved raw TAS script \"%s\".\n", fullRawPath.c_str());

	file.close();
}

std::string TasParser::SaveRawScriptToString(TasScript script) {
	
	std::ostringstream tasString;

	std::sort(script.processedFramebulks.begin(), script.processedFramebulks.end(), [](const TasFramebulk &a, const TasFramebulk &b) {
		return a.tick < b.tick;
	});

	tasString << "version " << std::to_string(script.header.version) << "\n";

	if (script.header.startInfo.isNext) tasString << "start next ";
	else tasString << "start ";

	switch (script.header.startInfo.type) {
	case TasScriptStartType::ChangeLevel:
		tasString << "map " << script.header.startInfo.param << "\n";
		break;
	case TasScriptStartType::LoadQuicksave:
		tasString << "save " << script.header.startInfo.param << "\n";
		break;
	case TasScriptStartType::StartImmediately:
		tasString << "now\n";
		break;
	case TasScriptStartType::ChangeLevelCM:
		tasString << "cm " << script.header.startInfo.param << "\n";
		break;
	}

	std::string prevInput = "";

	int last_tick = -1;

	for (TasFramebulk &fb : script.processedFramebulks) {
		std::string line = ">";

		line += Utils::ssprintf("%.*g %.*g|%.*g %.*g|", FLT_DECIMAL_DIG, fb.moveAnalog.x, FLT_DECIMAL_DIG, fb.moveAnalog.y, FLT_DECIMAL_DIG, fb.viewAnalog.x, FLT_DECIMAL_DIG, fb.viewAnalog.y);

		line += fb.buttonStates[TasControllerInput::Jump] ? "J" : "j";
		line += fb.buttonStates[TasControllerInput::Crouch] ? "D" : "d";
		line += fb.buttonStates[TasControllerInput::Use] ? "U" : "u";
		line += fb.buttonStates[TasControllerInput::Zoom] ? "Z" : "z";
		line += fb.buttonStates[TasControllerInput::FireBlue] ? "B" : "b";
		line += fb.buttonStates[TasControllerInput::FireOrange] ? "O" : "o";

		if (line == prevInput) {
			line = ">||";
		} else {
			prevInput = line;
		}

		line += "|";

		for (size_t i = 0; i < fb.commands.size(); ++i) {
			if (i != 0) line += ";";
			line += fb.commands[i];
		}

		if (line != ">|||") {
			last_tick = fb.tick;
			tasString << fb.tick << line << "\n";
		}
	}

	TasFramebulk &last = script.processedFramebulks[script.processedFramebulks.size() - 1];
	if (last.tick > last_tick) {
		// Make sure there's an empty bulk at the end so the TAS is the
		// right length
		tasString << last.tick << ">\n";
	}

	return tasString.str();
}
