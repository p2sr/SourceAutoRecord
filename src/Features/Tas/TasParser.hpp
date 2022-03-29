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
	void SaveFramebulksToFile(std::string name, TasStartInfo startInfo, bool wasStartNext, std::vector<TasFramebulk> framebulks);
	int toInt(std::string &str);
	float toFloat(std::string str);
};
