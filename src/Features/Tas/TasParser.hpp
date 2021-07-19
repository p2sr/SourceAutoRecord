#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "TasPlayer.hpp"

// shoutouts to Blender for making TAS script parsing.

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

namespace TasParser {
    std::vector<TasFramebulk> ParseFile(std::string filePath);
    bool ParseHeader(std::string filePath);
    std::vector<TasFramebulk> ParseAllLines(std::vector<std::string>& lines);
    RawFramebulk PreParseLine(std::string& line, const unsigned int lineNumber);
    TasFramebulk ParseRawFramebulk(RawFramebulk& bulk, TasFramebulk& previous);
    Vector ParseVector(std::string& str);
    std::vector<std::string> Tokenize(std::string& str, char separator = ' ');
    std::vector<std::string> ParseTool(std::string& str);
    int toInt(std::string& str);
    float toFloat(std::string str);

    void SaveFramebulksToFile(std::string name, TasStartInfo startInfo, std::vector<TasFramebulk> framebulks);
 };
