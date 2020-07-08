#include "TasParser.hpp"

#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>

#include "TasTools/StrafeTool.hpp"

std::regex const regexVector{ R"((?:(-?\d*\.?\d*)(?:\s)*(-?\d*\.?\d*))(?:\s)*)" };
std::regex const regexTool{ R"(\s+)" };
std::regex const regexNumber{ R"((-?\d*\.?\d*))" };

TasParser::TasParser()
{
}

std::vector<TasFramebulk> TasParser::ParseFile(std::string filePath)
{
    std::ifstream file(filePath, std::fstream::in);
    if (!file) {
        throw TasParserException("Failed to open the file");
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line))
        lines.push_back(line);

    file.close();
    std::vector<TasFramebulk> fb;

    try {
        fb = ParseAllLines(lines);
    } catch (TasParserException& e) {
        throw TasParserException(e.what());
    }

    return fb;
}

std::vector<TasFramebulk> TasParser::ParseAllLines(std::vector<std::string>& lines)
{
    std::vector<RawFramebulk> raws;
    std::vector<TasFramebulk> bulks;

    int lineCounter = 1;
    for (auto& line : lines) {
        if (line.empty() || (line.size() > 2 && line[0] == '/' && line[1] == '/')) { //Commentary
            ++lineCounter;
            continue;
        }

        try {
            auto raw = TasParser::PreParseLine(line, lineCounter);
            raws.push_back(raw);
        } catch (TasParserException& e) {
            throw TasParserException("(" + std::string(e.what()) + ") at line " + std::to_string(lineCounter));
        }

        ++lineCounter;
    }
    std::sort(raws.begin(), raws.end(), [](RawFramebulk a, RawFramebulk b) { return b.tick > b.tick; });

    if (raws.empty())
        return bulks;

    TasFramebulk bulk;

    try {
        bulks.push_back(TasParser::ParseRawFramebulk(raws[0], bulk));
    } catch (TasParserException& e) {
        throw TasParserException("(" + std::string(e.what()) + ") at line " + std::to_string(raws[0].lineNumber));
    }

    for (int i = 0; i < raws.size(); ++i) {
        try {
            bulks.push_back(TasParser::ParseRawFramebulk(raws[i], *--bulks.end()));
        } catch (TasParserException& e) {
            throw TasParserException("(" + std::string(e.what()) + ") at line " + std::to_string(raws[i].lineNumber));
        }
    }

    return bulks;
}

RawFramebulk TasParser::PreParseLine(std::string& line, const unsigned int lineNumber)
{
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
TasFramebulk TasParser::ParseRawFramebulk(RawFramebulk& raw, TasFramebulk& previous)
{
    TasFramebulk bulk = previous;
    bulk.tick = raw.tick;
    bulk.commands.clear();
    bulk.toolCmds.clear();

    std::stringstream ss(raw.raw);
    std::string tmp;

    int counter = 0;
    while (std::getline(ss, tmp, '|')) {
        if (tmp.empty() || tmp.find_first_not_of(' ') == tmp.npos) { //Check if empty or only space
            ++counter;
            continue;
        }

        //frame>movement|rotation|buttons|commands|tools
        switch (counter) {
        case 0: //Movement
            bulk.moveAnalog = TasParser::ParseVector(tmp);
            break;
        case 1: //Rotation
            bulk.viewAnalog = TasParser::ParseVector(tmp);
            break;
        case 2: //Buttons : jduzbo -> jump, duck, use, zoom, blue, orange
        {
            for (auto& c : tmp) {
                switch (c) {
                case 'J':
                    bulk.buttonStates[0] = true;
                    break;
                case 'j':
                    bulk.buttonStates[0] = false;
                    break;
                case 'D':
                    bulk.buttonStates[1] = true;
                    break;
                case 'd':
                    bulk.buttonStates[1] = false;
                    break;
                case 'U':
                    bulk.buttonStates[2] = true;
                    break;
                case 'u':
                    bulk.buttonStates[2] = false;
                    break;
                case 'Z':
                    bulk.buttonStates[3] = true;
                    break;
                case 'z':
                    bulk.buttonStates[3] = false;
                    break;
                case 'B':
                    bulk.buttonStates[4] = true;
                    break;
                case 'b':
                    bulk.buttonStates[4] = false;
                    break;
                case 'O':
                    bulk.buttonStates[5] = true;
                    break;
                case 'o':
                    bulk.buttonStates[5] = false;
                    break;
                default:
                    break;
                }
            }
        } break;
        case 3: //Commands
        {
            std::stringstream ss2(tmp);
            std::string tmp2;

            while (std::getline(ss2, tmp2, ' '))
                bulk.commands.push_back(tmp2);
        } break;
        case 4: //Tools. ex: strafe move; autojump on
        {
            std::for_each(tmp.begin(), tmp.end(), [](char& c) { c = std::tolower(c); });

            auto tools = TasParser::Tokenize(tmp, ';'); //tools = {"strafe move", "autojump on"}
            for (auto& toolAndParams : tools) //toolAndParams = "strafe move"
            {
                auto tokens = TasParser::ParseTool(toolAndParams); //tokens = {"strafe", "move"}

                for (auto& tool : TasTool::GetList()) {
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
                            bulk.toolCmds.push_back({ tool->GetTool(), cmds });
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

Vector TasParser::ParseVector(std::string& str)
{
    float x = 0, y = 0;
    std::smatch matches;
    if (std::regex_search(str, matches, regexVector)) {
        try {
            x = TasParser::toFloat(matches[1].str()); //The first sub_match is the whole string; the next sub_match is the first parenthesized expression.
            y = TasParser::toFloat(matches[2].str());
        } catch (...) {
            throw TasParserException("Can't parse vector { " + str + " }");
        }
    } else {
        //Handle error
        throw TasParserException("Can't parse vector { " + str + " }");
    }

    return Vector{ x, y };
}

std::vector<std::string> TasParser::Tokenize(std::string& str, char separator)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string tmp;

    while (std::getline(ss, tmp, separator))
        tokens.push_back(tmp);

    return tokens;
}

std::vector<std::string> TasParser::ParseTool(std::string& str)
{
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

int TasParser::toInt(std::string& str)
{
    std::smatch matches;
    if (std::regex_match(str, matches, regexNumber)) { //If it's a number
        return std::stoi(str);
    } else {
        throw TasParserException(str + " is not a number");
    }
}

float TasParser::toFloat(std::string& str)
{
    std::smatch matches;
    if (std::regex_match(str, matches, regexNumber)) { //If it's a number
        return std::stof(str);
    } else {
        throw TasParserException(str + " is not a number");
    }
}

bool TasParser::isNumber(std::string& str)
{
    return std::regex_match(str, regexNumber);
}
