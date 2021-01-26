#pragma once

#include "Command.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"

struct Rect {
    Vector a;
    Vector b;
    Vector c;
    Vector d;
    Vector e;
    Vector f;
    Vector g;
    Vector h;

    bool show;
};

class ZachStats {

public:
    ZachStats();
    void UpdateRects();
    void AddRect(Vector& a, Vector& B);
    std::vector<Rect>& GetRects();

private:
    void CheckRects(Vector& pos);
};

extern ZachStats* zachStats;

extern Command sar_stats_rect;