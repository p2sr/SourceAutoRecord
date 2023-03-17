#pragma once
#include "TasPlayer.hpp"

#include <iostream>
#include <string>
#include <vector>

// shoutouts to Blender for making TAS script parsing.

struct TasParserException : public std::exception {
	std::string msg;
	TasParserException(std::string msg)
		: msg(msg) {
	}
	~TasParserException() throw() {}
	const char *what() const throw() { return msg.c_str(); }
};

namespace TasParser {
	std::vector<TasFramebulk> ParseFile(int slot, std::string filePath);
	std::vector<TasFramebulk> ParseScript(int slot, std::string scriptName, std::string scriptString);
	void SaveFramebulksToFile(std::string name, TasStartInfo startInfo, bool wasStartNext, int version, std::vector<TasFramebulk> framebulks);
	std::string SaveFramebulksToString(TasStartInfo startInfo, bool wasStartNext, int version, std::vector<TasFramebulk> framebulks);
	int toInt(std::string &str);
	float toFloat(std::string str);
};
