#pragma once
#include <Features/Tas/TasController.hpp>
#include <Features/Tas/TasTool.hpp>


enum TasScriptStartType {
	ChangeLevel,
	ChangeLevelCM,
	LoadQuicksave,
	StartImmediately,
};

struct TasScriptStartInfo {
	bool isNext;
	TasScriptStartType type;
	std::string param;
};

struct TasScriptHeader {
	int version;
	TasScriptStartInfo startInfo;
	std::string rngManipFile;
};

struct TasFramebulk {
	int tick = 0;
	Vector moveAnalog = {0, 0};
	Vector viewAnalog = {0, 0};
	bool buttonStates[TAS_CONTROLLER_INPUT_COUNT] = {0};
	std::vector<std::string> commands;
	std::vector<TasToolCommand> toolCmds;

	std::string ToString();
};


struct TasScript {
	std::string name;
	std::string path;
	TasScriptHeader header;
	std::vector<TasFramebulk> framebulks;
	
	bool loadedFromFile;
	bool forceRawPlayback = false;
	std::vector<TasFramebulk> processedFramebulks;
	std::vector<std::string> userCmdDebugs;
	std::vector<std::string> playerInfoDebugs;
	
	inline bool IsRaw() const {
		return forceRawPlayback || (name.length() > 0 && name.find("_raw") != std::string::npos);
	}
	inline bool IsActive() const { return framebulks.size() > 0; }

	void ClearGeneratedContent();
};