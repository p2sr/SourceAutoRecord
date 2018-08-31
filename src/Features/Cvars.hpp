#pragma once
#include "Feature.hpp"

#include <fstream>

#include "Utils/SDK.hpp"

class Cvars : public Feature {
public:
    Cvars();
    void ListAll();
    int Dump(std::ofstream& file);
    void PrintHelp(const CCommand& args);
};

extern Cvars* cvars;
