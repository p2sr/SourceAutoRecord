#pragma once
#include "TasScript.hpp"

#define MAX_SCRIPT_VERSION 8

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
	TasScript ParseFile(std::string filePath);
	TasScript ParseScript(std::string scriptName, std::string scriptString);
	void SaveRawScriptToFile(TasScript script);
	std::string SaveRawScriptToString(TasScript script);
	int toInt(std::string &str);
	float toFloat(std::string str);
};
