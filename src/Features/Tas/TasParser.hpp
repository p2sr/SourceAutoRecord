#pragma once
#include "TasScript.hpp"

#define MAX_SCRIPT_VERSION 9

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

struct TasParserArgumentCountException : public TasParserException {
	TasParserArgumentCountException(TasTool* tool, int count)
		: TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", tool->GetName(), count)) {
	}
};

struct TasParserArgumentException : public TasParserException {
	TasParserArgumentException(TasTool* tool, std::string paramName, std::string arg)
		: TasParserException(Utils::ssprintf("Wrong %s argument for tool %s: %s", paramName.c_str(), tool->GetName(), arg.c_str())) {
	}

	TasParserArgumentException(TasTool *tool, std::string arg)
		: TasParserException(Utils::ssprintf("Wrong argument for tool %s: %s", tool->GetName(), arg.c_str())) {
	}
};

namespace TasParser {
	TasScript ParseFile(TasScript &script, std::string filePath);
	TasScript ParseScript(TasScript &script, std::string scriptName, std::string scriptString);
	void SaveRawScriptToFile(TasScript script);
	std::string SaveRawScriptToString(TasScript script);
	int toInt(std::string &str);
	float toFloat(std::string str);
	bool hasSuffix(const std::string &str, const std::string &suffix);
	float toFloatAssumeSuffix(std::string str, const std::string &suffix);
};
