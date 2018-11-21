#pragma once
#include "Features/Feature.hpp"
#include "Utils/SDK.hpp"

class StatsExport : public Feature {

public:
    StatsExport();
    void Record();
    void Export(std::string filename);

public:
    std::vector<float> velocity;
    std::vector<float> acceleration;
    bool isrecording;

private:
    int m_last_tick;
};
extern StatsExport* statsExport;
