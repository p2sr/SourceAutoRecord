#pragma once

#include "Command.hpp"
#include "Utils/SDK.hpp"
#include "Utils.hpp"
#include "Variable.hpp"
#include "Features.hpp"

#include "Utils/Math.hpp"

#include <sstream>


class Box {
public:
    std::array<Vector, 2> origVerts;
    std::array<Vector, 8> verts;
    Vector origin;
    unsigned int ID;

    float angle = 0;

    void Rotate(double angle)
    {
        auto rad = DEG2RAD(angle);
        Matrix rot(3, 3, 0);
        rot(0, 0) = std::cos(rad);
        rot(0, 1) = -std::sin(rad);
        rot(1, 0) = std::sin(rad);
        rot(1, 1) = std::cos(rad);
        rot(2, 2) = 1;

        for (auto& v : this->verts) {
            v -= this->origin;
        }

        //Rotation

        for (auto& v : this->verts) {
            v = rot * v;
        }

        //Translation back

        for (auto& v : this->verts) {
            v += this->origin;
        }

        this->angle += angle;
    }
};

class ZachTrigger : public Box {

public:
    bool show = true;
    bool isInside = false;
    bool triggered = false;

    void Trigger(std::stringstream& output);
};

class ZachStats : public Feature {

public:
    ZachStats();
    void UpdateTriggers();
    bool CheckTriggers(ZachTrigger& trigger, Vector& pos);
    void AddTrigger(Vector& a, Vector& B, float angle, unsigned int ID);
    void DeleteTrigger(unsigned int ID);
    void DrawTrigger(ZachTrigger& trigger);
    void PreviewSecond();
    std::vector<ZachTrigger>& GetTriggers();
    ZachTrigger* GetTriggerByID(unsigned int ID);
    void ResetTriggers();
    void ExportTriggers();

    std::stringstream& GetStream() { return this->output; }
    void ResetStream() { this->output.clear(); }
    bool ExportCSV();


    void SetHeader(std::string& header) { this->header = header; };

    Matrix m;
    bool isFirstPlaced;
    Vector A;

private:

    int lastFrameDrawn;
    std::string header;
    std::stringstream output;
};

extern ZachStats* zachStats;

extern Variable sar_zach_file;
extern Variable sar_zach_name;
extern Variable sar_zach_show_triggers;
extern Command sar_zach_export;
extern Command sar_zach_reset;
extern Command sar_trigger;
extern Command sar_trigger_place;
extern Command sar_trigger_rotate;
extern Command sar_trigger_delete;
