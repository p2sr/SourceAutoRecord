#include "TasParser.hpp"

#include "TasTools/StrafeTool.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>

//FrenchSaves10Ticks

std::regex const regexVector{R"((?:(-?\d*\.?\d*)(?:\s)*(-?\d*\.?\d*))(?:\s)*)"};
std::regex const regexTool{R"(\s+)"};
std::regex const regexNumber{R"((-?\d*\.?\d*))"};

std::vector<TasFramebulk> TasParser::ParseFile(std::string filePath) {
	std::ifstream file(filePath, std::fstream::in);
	if (!file) {
		throw TasParserException("Failed to open the file");
	}

	std::vector<std::string> lines;
	std::string line;

	if (std::getline(file, line)) {
		if (!line.empty() && (line.size() > 2 && line[0] != '/' && line[1] != '/')) {  //Commentary
			try {
				if (!TasParser::ParseHeader(line)) {
					lines.push_back(line);
				}
			} catch (TasParserException &e) {
				throw;
			}
		}
	}

	while (std::getline(file, line))
		lines.push_back(line);

	file.close();
	std::vector<TasFramebulk> fb;

	try {
		fb = ParseAllLines(lines);
	} catch (TasParserException &e) {
		throw;
	}

	if (fb.size() > 0) {
		tasPlayer->SetLoadedFileName(filePath);
	}

	return fb;
}

bool TasParser::ParseHeader(std::string line) {
	std::stringstream ss(line);
	std::string tmp;
	if (!std::getline(ss, tmp))
		throw TasParserException("Can't parse header : " + line);

	std::for_each(tmp.begin(), tmp.end(), [](char &c) { c = tolower(c); });

	auto startParams = TasParser::Tokenize(tmp);  //startParams = {"start", "map", "sp_a1_intro1""}
	if (startParams.size() < 1 || startParams[0] != "start") {
		tasPlayer->SetStartInfo(TasStartType::UnknownStart, "");
		return false;
	}

	TasStartType type = TasStartType::UnknownStart;
	if (startParams.size() > 1) {
		if (startParams[1] == "map") {
			type = TasStartType::ChangeLevel;
		} else if (startParams[1] == "save") {
			type = TasStartType::LoadQuicksave;
		} else if (startParams[1] == "now") {
			type = TasStartType::StartImmediately;
		} else if (startParams[1] == "next") {
			type = TasStartType::WaitForNewSession;
		} else if (startParams[1] == "cm") {
			type = TasStartType::ChangeLevelCM;
		}
	}

	std::string param = "";
	if (startParams.size() > 2) {
		param = startParams[2];
	}

	tasPlayer->SetStartInfo(type, param);
	return true;
}

std::vector<TasFramebulk> TasParser::ParseAllLines(std::vector<std::string> &lines) {
	std::vector<RawFramebulk> raws;
	std::vector<TasFramebulk> bulks;

	int lineCounter = 1;
	int lastTick = -1;
	for (auto &line : lines) {
		if (line.empty() || (line.size() > 2 && line[0] == '/' && line[1] == '/')) {  //Commentary
			++lineCounter;
			continue;
		}

		try {
			auto raw = TasParser::PreParseLine(line, lineCounter);
			if (raw.tick <= lastTick) {
				throw TasParserException(Utils::ssprintf("Framebulk does not occur after previous framebulk tick (%d <= %d)", raw.tick, lastTick));
			}
			lastTick = raw.tick;
			raws.push_back(raw);
		} catch (TasParserException &e) {
			throw TasParserException("(" + std::string(e.what()) + ") at line " + std::to_string(lineCounter));
		}

		++lineCounter;
	}

	if (raws.empty())
		return bulks;

	TasFramebulk bulk;

	try {
		bulks.push_back(TasParser::ParseRawFramebulk(raws[0], bulk));
	} catch (TasParserException &e) {
		throw TasParserException("(" + std::string(e.what()) + ") at line " + std::to_string(raws[0].lineNumber));
	}

	for (int i = 0; i < raws.size(); ++i) {
		try {
			bulks.push_back(TasParser::ParseRawFramebulk(raws[i], *--bulks.end()));
		} catch (TasParserException &e) {
			throw TasParserException("(" + std::string(e.what()) + ") at line " + std::to_string(raws[i].lineNumber));
		}
	}

	return bulks;
}

RawFramebulk TasParser::PreParseLine(std::string &line, const unsigned int lineNumber) {
	RawFramebulk bulk;
	bulk.lineNumber = lineNumber;
	std::stringstream ss(line);
	std::string tmp;

	if (std::getline(ss, tmp, '>')) {
		bulk.tick = TasParser::toInt(tmp);
	} else {
		throw TasParserException("line \"" + line + "\"");
	}

	if (std::getline(ss, tmp)) {
		bulk.raw = tmp;
	} else {
		throw TasParserException("line \"" + line + "\"");
	}

	return bulk;
}

//For now, just assume tas script only contains framebulk
TasFramebulk TasParser::ParseRawFramebulk(RawFramebulk &raw, TasFramebulk &previous) {
	TasFramebulk bulk = previous;
	bulk.tick = raw.tick;
	bulk.commands.clear();
	bulk.toolCmds.clear();

	std::stringstream ss(raw.raw);
	std::string tmp;

	int counter = 0;
	while (std::getline(ss, tmp, '|')) {
		if (tmp.empty() || tmp.find_first_not_of(' ') == tmp.npos) {  //Check if empty or only space
			++counter;
			continue;
		}

		//frame>movement|rotation|buttons|commands|tools
		switch (counter) {
		case 0:  //Movement
			bulk.moveAnalog = TasParser::ParseVector(tmp);
			break;
		case 1:  //Rotation
			bulk.viewAnalog = TasParser::ParseVector(tmp);
			break;
		case 2:  //Buttons : jduzbo -> jump, duck, use, zoom, blue, orange
		{
			for (auto &c : tmp) {
				switch (c) {
				case 'J':
					bulk.buttonStates[TasControllerInput::Jump] = true;
					break;
				case 'j':
					bulk.buttonStates[TasControllerInput::Jump] = false;
					break;
				case 'D':
					bulk.buttonStates[TasControllerInput::Crouch] = true;
					break;
				case 'd':
					bulk.buttonStates[TasControllerInput::Crouch] = false;
					break;
				case 'U':
					bulk.buttonStates[TasControllerInput::Use] = true;
					break;
				case 'u':
					bulk.buttonStates[TasControllerInput::Use] = false;
					break;
				case 'Z':
					bulk.buttonStates[TasControllerInput::Zoom] = true;
					break;
				case 'z':
					bulk.buttonStates[TasControllerInput::Zoom] = false;
					break;
				case 'B':
					bulk.buttonStates[TasControllerInput::FireBlue] = true;
					break;
				case 'b':
					bulk.buttonStates[TasControllerInput::FireBlue] = false;
					break;
				case 'O':
					bulk.buttonStates[TasControllerInput::FireOrange] = true;
					break;
				case 'o':
					bulk.buttonStates[TasControllerInput::FireOrange] = false;
					break;
				default:
					break;
				}
			}
		} break;
		case 3:  //Commands
		{
			std::stringstream ss2(tmp);
			std::string tmp2;

			while (std::getline(ss2, tmp2, ';'))
				bulk.commands.push_back(tmp2);
		} break;
		case 4:  //Tools. ex: strafe move; autojump on
		{
			std::for_each(tmp.begin(), tmp.end(), [](char &c) { c = tolower(c); });

			auto tools = TasParser::Tokenize(tmp, ';');          //tools = {"strafe move", "autojump on"}
			for (auto &toolAndParams : tools) {                  //toolAndParams = "strafe move"
				auto tokens = TasParser::ParseTool(toolAndParams);  //tokens = {"strafe", "move"}

				if (tokens.size() == 0) continue;

				for (auto &tool : TasTool::GetList()) {
					if (tool->GetName() == tokens[0]) {
						std::vector<std::string> args;
						if (tokens.size() > 1) {
							args = std::vector(tokens.begin() + 1, tokens.end());
						} else {
							args = {};
						}

						auto cmds = tool->GetTool()->ParseParams(args);
						if (cmds == nullptr) {
							throw TasParserException("Bad parameters for " + std::string(tool->GetName()) + " tool !");
						} else {
							bulk.toolCmds.push_back({tool->GetTool(), cmds});
						}
					}
				}
			}
		} break;
		default:
			break;
		}

		++counter;
	}

	return bulk;
}

Vector TasParser::ParseVector(std::string &str) {
	float x = 0, y = 0;
	const char *vec = str.c_str();

	char *pEnd;
	x = std::strtof(vec, &pEnd);
	if (vec == pEnd) {  //If no conversion
		throw TasParserException("Can't parse vector { " + str + " }");
	}

	char *pEndY = pEnd;
	y = std::strtof(pEndY, &pEnd);
	if (pEndY == pEnd) {  //If no conversion
		throw TasParserException("Can't parse vector { " + str + " }");
	}

	return Vector(x, y, 0);
}

std::vector<std::string> TasParser::Tokenize(std::string &str, char separator) {
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string tmp;

	while (std::getline(ss, tmp, separator))
		tokens.push_back(tmp);

	return tokens;
}

std::vector<std::string> TasParser::ParseTool(std::string &str) {
	std::vector<std::string> tokens;
	std::sregex_token_iterator i(str.begin(), str.end(), regexTool, -1);
	std::sregex_token_iterator end;

	while (i != end) {
		if (i->matched)
			tokens.push_back(i->str());
		++i;
	}

	return tokens;
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


void TasParser::SaveFramebulksToFile(std::string name, TasStartInfo startInfo, std::vector<TasFramebulk> framebulks) {
	std::string fixedName = name;
	size_t lastdot = name.find_last_of(".");
	if (lastdot != std::string::npos) {
		fixedName = name.substr(0, lastdot);
	}

	std::ofstream file(fixedName + "_raw." + TAS_SCRIPT_EXT);

	std::sort(framebulks.begin(), framebulks.end(), [](const TasFramebulk &a, const TasFramebulk &b) {
		return a.tick < b.tick;
	});

	switch (startInfo.type) {
	case TasStartType::ChangeLevel:
		file << "start map " << startInfo.param << "";
		break;
	case TasStartType::LoadQuicksave:
		file << "start save " << startInfo.param << "";
		break;
	case TasStartType::StartImmediately:
		file << "start now";
		break;
	case TasStartType::ChangeLevelCM:
		file << "start cm";
		break;
	default:
		file << "start next";
		break;
	}

	for (TasFramebulk &fb : framebulks) {
		file << "\n";
		file << fb.tick << ">";

		char analogs[128];
		snprintf(analogs, sizeof(analogs), "%.9f %.9f|%.9f %.9f|", fb.moveAnalog.x, fb.moveAnalog.y, fb.viewAnalog.x, fb.viewAnalog.y);
		file << analogs;

		file << (fb.buttonStates[TasControllerInput::Jump] ? "J" : "j");
		file << (fb.buttonStates[TasControllerInput::Crouch] ? "D" : "d");
		file << (fb.buttonStates[TasControllerInput::Use] ? "U" : "u");
		file << (fb.buttonStates[TasControllerInput::Zoom] ? "Z" : "z");
		file << (fb.buttonStates[TasControllerInput::FireBlue] ? "B" : "b");
		file << (fb.buttonStates[TasControllerInput::FireOrange] ? "O" : "o");

		file << "|";

		for (std::string command : fb.commands) {
			file << command << ";";
		}
	}

	file.close();
}
