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

	inline bool operator==(const TasScriptHeader& rhs) {
		return this->version == rhs.version
			&& this->startInfo.isNext == rhs.startInfo.isNext
			&& this->startInfo.type == rhs.startInfo.type
			&& this->startInfo.param == rhs.startInfo.param
			&& this->rngManipFile == rhs.rngManipFile;
	}
};

struct TasFramebulk {
	int tick = 0;
	Vector moveAnalog = {0, 0};
	Vector viewAnalog = {0, 0};
	std::array<bool, TAS_CONTROLLER_INPUT_COUNT> buttonStates = {};
	std::vector<std::string> commands;
	std::vector<TasToolCommand> toolCmds;

	std::string ToString();

	inline bool operator==(const TasFramebulk& rhs) {
		return this->toolCmds == rhs.toolCmds
			&& std::tie(this->tick, this->moveAnalog, this->viewAnalog, this->buttonStates, this->commands)
			== std::tie(rhs.tick, rhs.moveAnalog, rhs.viewAnalog, rhs.buttonStates, rhs.commands);
	}
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
	int FirstDifference(TasScript* other);
};
