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

    Box(const Vector& A, const Vector& G, unsigned ID, double angle = 0)
    {
        this->origVerts = {A, G};
        this->origin = (A + G) / 2;
        this->SetRotation(angle); // Initializes 'verts' for us too
        this->ID = ID;
    }

    void SetRotation(double angle)
    {
        Vector &A = this->origVerts[0], &G = this->origVerts[1];

        float lengthX = G.x - A.x;
        float lengthY = G.y - A.y;
        float lengthZ = G.z - A.z;

        Vector B{ A.x + lengthX, A.y, A.z };
        Vector C{ A.x + lengthX, A.y + lengthY, A.z };
        Vector D{ A.x, A.y + lengthY, A.z };

        Vector E{ A.x, A.y, A.z + lengthZ };
        Vector F{ A.x + lengthX, A.y, A.z + lengthZ };
        Vector H{ A.x, A.y + lengthY, A.z + lengthZ };

        // "Why do we reassign this instead of just calculating a
        // rotation delta" I hear you ask? Well, if we calculated a
        // rotation delta, there would be floating-point errors, and as
        // the rotation was set, the effect would get more and more.
        // It's probably not a *massive* deal, but it's a thing that
        // came to mind, and the recreation is relatively inexpensive
        this->verts = { A, B, C, D, E, F, G, H };

        // Rotate each one

        auto rad = DEG2RAD(angle);
        Matrix rot(3, 3, 0);
        rot(0, 0) = std::cos(rad);
        rot(0, 1) = -std::sin(rad);
        rot(1, 0) = std::sin(rad);
        rot(1, 1) = std::cos(rad);
        rot(2, 2) = 1;

        for (auto& v : this->verts) {
            v -= this->origin;
            v = rot * v;
            v += this->origin;
        }

        this->angle = angle;
    }
};

class ZachTrigger : public Box {
public:
    ZachTrigger(const Vector& A, const Vector& G, unsigned ID, double angle = 0)
        : Box::Box(A, G, ID, angle)
    { }
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
extern Command sar_zach_trigger_add;
extern Command sar_zach_trigger_place;
extern Command sar_zach_trigger_rotate;
extern Command sar_zach_trigger_delete;
