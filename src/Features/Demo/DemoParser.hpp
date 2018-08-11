#pragma once
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
    void Adjust(Demo* demo);
    bool Parse(std::string filePath, Demo* demo);
};
