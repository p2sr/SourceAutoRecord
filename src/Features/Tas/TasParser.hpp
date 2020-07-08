#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "TasPlayer.hpp"

// shoutouts to Blender for making TAS script parsing :)

struct TasParserException : public std::exception {
    std::string msg;
    TasParserException(std::string msg)
        : msg(msg)
    {
    }
    ~TasParserException() throw() {}
    const char* what() const throw() { return msg.c_str(); }
};

struct RawFramebulk {
    unsigned int lineNumber;
    int tick;
    std::string raw;
};

class TasParser {
public:
    TasParser();

    static std::vector<TasFramebulk> ParseFile(std::string filePath);
    static std::vector<TasFramebulk> ParseAllLines(std::vector<std::string>& lines);
    static RawFramebulk PreParseLine(std::string& line, const unsigned int lineNumber);
    static TasFramebulk ParseRawFramebulk(RawFramebulk& bulk, TasFramebulk& previous);
    static Vector ParseVector(std::string& str);
    static std::vector<std::string> Tokenize(std::string& str, char separator = ' ');
    static std::vector<std::string> ParseTool(std::string& str);
    static int toInt(std::string& str);
    static float toFloat(std::string& str);
    static bool isNumber(std::string& str);
};
