#pragma once
#include <fstream>

#include "Feature.hpp"

#include "Utils/SDK.hpp"

class Cvars : public Feature {
public:
    Cvars();
    void ListAll();
    int Dump(std::ofstream& file);
    void PrintHelp(const CCommand& args);
};

extern Cvars* cvars;
