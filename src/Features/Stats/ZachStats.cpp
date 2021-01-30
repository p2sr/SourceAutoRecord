#include "ZachStats.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include <cmath>

#ifdef _WIN32
#define PLAT_CALL(fn, ...) fn(__VA_ARGS__)
#else
#define PLAT_CALL(fn, ...) fn(nullptr, __VA_ARGS__)
#endif

DECL_CVAR_CALLBACK(sar_zach_name)
{
    auto& stream = zachStats->GetStream();
    if (stream.tellp() != std::streampos(0))
        stream << "\n";
    stream << sar_zach_name.GetString();
}

Variable sar_zach_file("sar_zach_file", "zach.csv", "Name of the file to export.\n", 0);
Variable sar_zach_name("sar_zach_name", "FrenchSaves10ticks", "Name of the current player.\n", 0, &sar_zach_name_callback);
Variable sar_zach_show_triggers("sar_zach_show_triggers", "1", "Draw the triggers in-game");

//plugin_load sar; sar_shane_loads 1; sar_disable_progress_bar_update 2; bind mouse5 "sar_trigger_place 1"

//Triggers

void ZachTrigger::Trigger(std::stringstream& output)
{
    console->Print("%s -> %d (%.3f)\n", sar_zach_name.GetString(), session->GetTick(), session->GetTick() / 60.f);
    output << CSV_SEPARATOR << session->GetTick() << " (" << session->GetTick() / 60.f << ")";
    this->triggered = true;
}

//Stats

ZachStats* zachStats;

ZachStats::ZachStats()
    : m(3, 3, 0)
    , output(MICROSOFT_PLEASE_FIX_YOUR_SOFTWARE_SMHMYHEAD "\n")
    , header("Triggers,Door,Floor breaks,Blue Portal,Office window")
    , isFirstPlaced(false)
    , lastFrameDrawn(0)
{
    this->hasLoaded = true;
}

void ZachStats::UpdateTriggers()
{
    auto player = client->GetPlayer(GET_SLOT() + 1);
    if (!player)
        return;

    auto pos = client->GetAbsOrigin(player);

    for (auto& trigger : this->GetTriggers()) {
        if (sar_zach_show_triggers.GetBool() && this->lastFrameDrawn + 60 <= session->GetTick()) {
            this->DrawTrigger(trigger);
        }

        if (this->CheckTriggers(trigger, pos)) {
            trigger.Trigger(this->output);
        }
    }

    if (sar_zach_show_triggers.GetBool() && this->lastFrameDrawn + 60 <= session->GetTick()) {
        this->lastFrameDrawn = session->GetTick();
    }

    if (this->isFirstPlaced) {
        this->PreviewSecond();
    }
}

void ZachStats::AddTrigger(Vector& A, Vector& G, float angle, unsigned int ID)
{
    // 'A' must have minimum coordinates, 'G' must have max
#define SWAP(a,b) do { float tmp = a; a = b; b = tmp; } while (0)
    if (A.x > G.x) SWAP(A.x, G.x);
    if (A.y > G.y) SWAP(A.y, G.y);
    if (A.z > G.z) SWAP(A.z, G.z);
#undef SWAP
    auto trigger = this->GetTriggerByID(ID);
    if (trigger == nullptr) { //Trigger ID doesn't exist

        float lengthX = G.x - A.x;
        float lengthY = G.y - A.y;
        float lengthZ = G.z - A.z;

        Vector B{ A.x + lengthX, A.y, A.z };
        Vector C{ A.x + lengthX, A.y + lengthY, A.z };
        Vector D{ A.x, A.y + lengthY, A.z };

        Vector E{ A.x, A.y, A.z + lengthZ };
        Vector F{ A.x + lengthX, A.y, A.z + lengthZ };
        Vector H{ A.x, A.y + lengthY, A.z + lengthZ };

        //"mouse5" = "sar_stats_rect -150 -400 960 -82 -331 1003"

        Vector origin{ (G.x + A.x) / 2, (G.y + A.y) / 2, (G.z + A.z) / 2 };

        ZachTrigger trigger{ { {A, G}, {A, B, C, D, E, F, G, H}, origin, ID } };

        trigger.Rotate(angle);

        this->GetTriggers().push_back(trigger);
        console->Print("Trigger added\n");
    } else { //There's already a trigger with that ID
        auto oldAngle = trigger->angle;

        this->DeleteTrigger(ID);
        this->AddTrigger(A, G, oldAngle, ID);
    }
}

void ZachStats::DeleteTrigger(unsigned int ID)
{
    auto& v = this->GetTriggers();
    unsigned int idx = 0;
    for (auto& it : v) {
        if (it.ID == ID) {
            v.erase(v.begin() + idx);
            break;
        }
        ++idx;
    }
}

void ZachStats::DrawTrigger(ZachTrigger& trigger)
{
    PLAT_CALL(engine->AddLineOverlay, trigger.verts[0], trigger.verts[1], 255, 0, 0, false, 1);
    PLAT_CALL(engine->AddLineOverlay, trigger.verts[0], trigger.verts[3], 255, 0, 0, false, 1);
    PLAT_CALL(engine->AddLineOverlay, trigger.verts[3], trigger.verts[2], 255, 0, 0, false, 1);
    PLAT_CALL(engine->AddLineOverlay, trigger.verts[2], trigger.verts[1], 255, 0, 0, false, 1);

    PLAT_CALL(engine->AddLineOverlay, trigger.verts[4], trigger.verts[7], 255, 0, 0, false, 1);
    PLAT_CALL(engine->AddLineOverlay, trigger.verts[4], trigger.verts[5], 255, 0, 0, false, 1);
    PLAT_CALL(engine->AddLineOverlay, trigger.verts[7], trigger.verts[6], 255, 0, 0, false, 1);
    PLAT_CALL(engine->AddLineOverlay, trigger.verts[5], trigger.verts[6], 255, 0, 0, false, 1);

    PLAT_CALL(engine->AddLineOverlay, trigger.verts[0], trigger.verts[4], 255, 0, 0, false, 1);
    PLAT_CALL(engine->AddLineOverlay, trigger.verts[3], trigger.verts[7], 255, 0, 0, false, 1);
    PLAT_CALL(engine->AddLineOverlay, trigger.verts[2], trigger.verts[6], 255, 0, 0, false, 1);
    PLAT_CALL(engine->AddLineOverlay, trigger.verts[1], trigger.verts[5], 255, 0, 0, false, 1);

    //const Vector& origin, const Vector& mins, const Vector& MAX, QAngle const& orientation, int r, int g, int b, int a, float duration
    //Vector origin = trigger.origin;

    //PLAT_CALL(engien->AddBoxOverlay, )
}

void ZachStats::PreviewSecond()
{
    //Trace ray to place the 2nd point
    CGameTrace tr;
    engine->TraceFromCamera(65535.0, tr);

    auto const& G = tr.endpos;

    //Draw the box
    float lengthX = G.x - A.x;
    float lengthY = G.y - A.y;
    float lengthZ = G.z - A.z;

    Vector B{ A.x + lengthX, A.y, A.z };
    Vector C{ A.x + lengthX, A.y, A.z + lengthZ };
    Vector D{ A.x, A.y, A.z + lengthZ };

    Vector E{ A.x, A.y + lengthY, A.z };
    Vector F{ A.x + lengthX, A.y + lengthY, A.z };
    Vector H{ A.x, A.y + lengthY, A.z + lengthZ };

    Vector origin{ (G.x + A.x) / 2, (G.y + A.y) / 2, (G.z + A.z) / 2 };

    PLAT_CALL(engine->AddLineOverlay, A, B, 255, 0, 0, false, 0);
    PLAT_CALL(engine->AddLineOverlay, A, D, 255, 0, 0, false, 0);
    PLAT_CALL(engine->AddLineOverlay, D, C, 255, 0, 0, false, 0);
    PLAT_CALL(engine->AddLineOverlay, C, B, 255, 0, 0, false, 0);

    PLAT_CALL(engine->AddLineOverlay, E, H, 255, 0, 0, false, 0);
    PLAT_CALL(engine->AddLineOverlay, E, F, 255, 0, 0, false, 0);
    PLAT_CALL(engine->AddLineOverlay, H, G, 255, 0, 0, false, 0);
    PLAT_CALL(engine->AddLineOverlay, F, G, 255, 0, 0, false, 0);

    PLAT_CALL(engine->AddLineOverlay, A, E, 255, 0, 0, false, 0);
    PLAT_CALL(engine->AddLineOverlay, D, H, 255, 0, 0, false, 0);
    PLAT_CALL(engine->AddLineOverlay, C, G, 255, 0, 0, false, 0);
    PLAT_CALL(engine->AddLineOverlay, B, F, 255, 0, 0, false, 0);
}

std::vector<ZachTrigger>& ZachStats::GetTriggers()
{
    static std::vector<ZachTrigger> rects;
    return rects;
}

ZachTrigger* ZachStats::GetTriggerByID(unsigned int ID)
{
    for (auto& trigger : this->GetTriggers())
        if (trigger.ID == ID)
            return &trigger;
    return nullptr;
}

void ZachStats::ResetTriggers()
{
    for (auto& trigger : this->GetTriggers()) {
        trigger.isInside = false;
        trigger.triggered = false;
        this->lastFrameDrawn = 0;
    }
}

void ZachStats::ExportTriggers()
{
    std::string filePath = sar_zach_file.GetString();
    if (filePath.substr(filePath.length() - 4, 4) != ".cfg")
        filePath += ".cfg";

    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.good()) {
        file.close();
        return console->Print("Can't export the file.\n");
    }

    file << "//Explanation : sar_trigger A.x A.y A.z B.x B.y B.z angle ID" << std::endl;

    for (auto& trigger : this->GetTriggers()) {
        file << "sar_trigger " << trigger.origVerts[0].x << " " << trigger.origVerts[0].y << " " << trigger.origVerts[0].z
             << " " << trigger.origVerts[1].x << " " << trigger.origVerts[1].y << " " << trigger.origVerts[1].z
             << " " << trigger.angle
             << " " << trigger.ID
             << std::endl;
    }

    file.close();
    console->Print("Successfully exported to %s.\n", sar_zach_file.GetString());
}

bool ZachStats::ExportCSV()
{
    std::string filePath = sar_zach_file.GetString();
    if (filePath.substr(filePath.length() - 4, 4) != ".csv")
        filePath += ".csv";

    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.good()) {
        file.close();
        return false;
    }

    file << MICROSOFT_PLEASE_FIX_YOUR_SOFTWARE_SMHMYHEAD << std::endl;
    file << this->header << std::endl;
    file << this->output.str() << std::endl;

    file.close();
    return true;
}

bool ZachStats::CheckTriggers(ZachTrigger& trigger, Vector& pos)
{
    if (trigger.triggered) {
        return false;
    }

    // Step 1: collide on z (vertical). As the box is effectively axis-aligned on
    // this axis, we can just compare the point's z coordinate to the top
    // and bottom of the cuboid

    float zMin = trigger.verts[0].z, zMax = trigger.verts[4].z; // A point on the bottom and top respectively
    if (pos.z < zMin || pos.z > zMax) {
        /*if (trigger.ID == 1) {
            console->Print("triggered %d\n", trigger.ID);
        }*/
        return false;
    }

    // Step 2: collide on x and y. We can now ignore the y axis entirely,
    // and instead look at colliding the point {p.x, p.y} with the
    // rectangle defined by the top or bottom face of the cuboid

    // Algorithm stolen from https://stackoverflow.com/a/2752754/13932065

    float bax = trigger.verts[1].x - trigger.verts[0].x;
    float bay = trigger.verts[1].y - trigger.verts[0].y;
    float dax = trigger.verts[3].x - trigger.verts[0].x;
    float day = trigger.verts[3].y - trigger.verts[0].y;

    if ((pos.x - trigger.verts[0].x) * bax + (pos.y - trigger.verts[0].y) * bay < 0) {
        if (trigger.ID == 1) {
        }
        return false;
    }
    if ((pos.x - trigger.verts[1].x) * bax + (pos.y - trigger.verts[1].y) * bay > 0) {
        if (trigger.ID == 1) {
        }
        return false;
    }
    if ((pos.x - trigger.verts[0].x) * dax + (pos.y - trigger.verts[0].y) * day < 0) {
        if (trigger.ID == 1) {
        }
        return false;
    }
    if ((pos.x - trigger.verts[3].x) * dax + (pos.y - trigger.verts[3].y) * day > 0) {
        if (trigger.ID == 1) {
        }
        return false;
    }

    return true;
}

CON_COMMAND(sar_trigger, "test")
{
    if (args.ArgC() < 8) {
        return console->Print(sar_trigger.ThisPtr()->m_pszHelpString);
    }

    Vector A = Vector(std::atof(args[1]), std::atof(args[2]), std::atof(args[3]));
    Vector G = Vector(std::atof(args[4]), std::atof(args[5]), std::atof(args[6]));

    unsigned int ID = 0;
    float angle = 0;
    if (args.ArgC() >= 9) {
        angle = std::atof(args[7]);
        ID = std::atoi(args[8]);
    } else {
        ID = std::atoi(args[7]);
    }

    zachStats->AddTrigger(A, G, angle, ID);
}

CON_COMMAND(sar_trigger_place, "test")
{
    if (args.ArgC() < 2) {
        return console->Print(sar_trigger_place.ThisPtr()->m_pszHelpString);
    }

    CGameTrace tr;
    bool hit = engine->TraceFromCamera(65535.0f, tr);
    if (!hit)
        return console->Print("You aimed at the void.\n");

    if (!sar_zach_show_triggers.GetBool()) {
        console->Print("sar_zach_show_triggers set to 1 !\n");
        sar_zach_show_triggers.SetValue(1);
    }

    if (!zachStats->isFirstPlaced) {
        zachStats->A = tr.endpos;
        zachStats->isFirstPlaced = true;
    } else {
        zachStats->AddTrigger(zachStats->A, tr.endpos, 0.0f, std::atoi(args[1]));
        zachStats->isFirstPlaced = false;
    }
}

CON_COMMAND(sar_trigger_rotate, "test")
{
    if (args.ArgC() < 3) {
        return console->Print(sar_trigger_rotate.ThisPtr()->m_pszHelpString);
    }

    auto trigger = zachStats->GetTriggerByID(std::atoi(args[2]));
    if (trigger == nullptr)
        return console->Print("No trigger\n");

    trigger->Rotate(std::atoi(args[1]));
}

CON_COMMAND(sar_trigger_delete, "test")
{
    if (args.ArgC() < 2) {
        return console->Print(sar_trigger_delete.ThisPtr()->m_pszHelpString);
    }

    zachStats->DeleteTrigger(std::atoi(args[1]));
}

CON_COMMAND(sar_zach_export_stats, "test")
{
    zachStats->ExportCSV();
}

CON_COMMAND(sar_zach_export_triggers, "test")
{
    zachStats->ExportTriggers();
}

CON_COMMAND(sar_zach_reset, "test")
{
    zachStats->ResetTriggers();
    zachStats->ResetStream();
}
