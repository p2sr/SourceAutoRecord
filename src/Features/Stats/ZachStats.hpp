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

    float angle = 0;

    Box(const Vector& A, const Vector& G, double angle = 0)
    {
        this->origVerts = {A, G};
        this->origin = (A + G) / 2;
        this->SetRotation(angle); // Initializes 'verts' for us too
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

enum class TriggerType {
    ZYPEH,
    ZYNTEX
};

class Trigger {
public:
    Trigger(const unsigned int ID, TriggerType type)
        : ID(ID)
        , type(type)
    {
    }

    TriggerType type;
    bool triggered = false;
    unsigned int ID;
};

class ZypehTrigger : public Trigger, public Box {
public:
    ZypehTrigger(const Vector& A, const Vector& G, unsigned ID, double angle = 0)
        : Trigger(ID, TriggerType::ZYPEH)
        , Box::Box(A, G, angle)
    {
    }
};

class ZyntexTrigger : public Trigger {
public:
    ZyntexTrigger(const std::string entName, const std::string input, const unsigned int ID)
        : Trigger(ID, TriggerType::ZYNTEX)
        , entName(entName)
        , input(input)
    {
    }

    std::string entName;
    std::string input;
};

static std::vector<Trigger*> g_triggers;

class ZachStats : public Feature {

public:
    ZachStats();
    void UpdateTriggers();
    bool CheckZypehTriggers(ZypehTrigger* trigger, Vector& pos);
    void CheckZyntexTriggers(void* entity, const char* input);
    void AddZypehTrigger(Vector& a, Vector& B, float angle, unsigned int ID);
    void AddZyntexTrigger(const std::string entName, const std::string input, unsigned int ID);
    void DeleteTrigger(unsigned int ID);
    void DeleteAll();
    void DrawTrigger(ZypehTrigger* trigger);
    void PreviewSecond();
    std::vector<Trigger*>& GetTriggers();
    Trigger* GetTriggerByID(unsigned int ID);
    void ResetTriggers();
    bool ExportTriggers(std::string filePath);

    static void Output(std::stringstream& output, const float time);
    std::stringstream& GetStream() { return this->output; }
    void ResetStream() { this->output.str(""); }
    bool ExportCSV(std::string filename);

    void NewSession();

    bool isFirstPlaced;
    Vector A;
    int lastFrameDrawn;
    float lastTriggerSplit;

private:

    std::stringstream output;
};

extern ZachStats* zachStats;

extern Variable sar_mtrigger_name;
extern Variable sar_mtrigger_header;
extern Variable sar_mtrigger_show_chat;

extern Variable sar_mtrigger_draw;

extern Command sar_mtrigger_export_triggers;
extern Command sar_mtrigger_export_stats;
extern Command sar_mtrigger_reset;
extern Command sar_mtrigger_add;
extern Command sar_mtrigger_delete;
extern Command sar_mtrigger_delete_all;

extern Command sar_mtrigger_place;
extern Command sar_mtrigger_rotate;
