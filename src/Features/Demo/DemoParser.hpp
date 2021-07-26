#pragma once
#include "DemoGhostPlayer.hpp"
#include "Variable.hpp"

#include <map>
#include <string>

class Demo;

// Basic demo parser which can handle Portal 2 and Half-Life 2 demos
class DemoParser {
public:
	bool headerOnly;
	int outputMode;
	bool hasAlignmentByte;
	int maxSplitScreenClients;

public:
	DemoParser();
	void Adjust(Demo *demo);
	bool Parse(std::string filePath, Demo *demo, bool ghostRequest = false, std::map<int, DataGhost> *datas = nullptr);
};

extern Variable sar_time_demo_dev;
