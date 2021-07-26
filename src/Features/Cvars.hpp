#pragma once
#include "Feature.hpp"
#include "Utils/SDK.hpp"

#include <fstream>

class Cvars : public Feature {
private:
	bool locked;

public:
	Cvars();
	void ListAll();
	int Dump(std::ofstream &file);
	int DumpDoc(std::ofstream &file);
	void PrintHelp(const CCommand &args);
	void Lock();
	void Unlock();
};

extern Cvars *cvars;
