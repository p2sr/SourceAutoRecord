#include "ZachStats.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include <cmath>
#include <cstdlib>

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
Variable sar_zach_show_triggers("sar_zach_show_triggers", "1", "Draw the triggers in-game.\n");

//plugin_load sar; sar_shane_loads 1; sar_disable_progress_bar_update 2; bind mouse5 "sar_zach_trigger_place 1"

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

    this->DeleteTrigger(ID); // Make sure there's not already a trigger with that ID


    ZachTrigger trigger(A, G, ID, angle);

    this->GetTriggers().push_back(trigger);
    console->Print("Trigger added\n");
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
    PLAT_CALL(
        engine->AddBoxOverlay,
        trigger.origin,
        trigger.origVerts[0] - trigger.origin,
        trigger.origVerts[1] - trigger.origin,
        {0, trigger.angle, 0},
        255, 0, 0,
        false,
        1
    );
}

void ZachStats::PreviewSecond()
{
    //Trace ray to place the 2nd point
    CGameTrace tr;
    engine->TraceFromCamera(65535.0, tr);

    auto const& G = tr.endpos;

    //Draw the box

    Vector origin{ (G.x + A.x) / 2, (G.y + A.y) / 2, (G.z + A.z) / 2 };
    PLAT_CALL(
        engine->AddBoxOverlay,
        origin,
        A - origin,
        G - origin,
        {0, 0, 0},
        255, 0, 0,
        false,
        0
    );
}

std::vector<ZachTrigger>& ZachStats::GetTriggers()
{
    static std::vector<ZachTrigger> rects;
    return rects;
}

ZachTrigger* ZachStats::GetTriggerByID(unsigned int ID)
{
    for (auto& trigger : this->GetTriggers()) {
        if (trigger.ID == ID) {
            return &trigger;
        }
    }
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

    file << "// Explanation : sar_zach_trigger_add A.x A.y A.z B.x B.y B.z angle ID" << std::endl;

    for (auto& trigger : this->GetTriggers()) {
        file << "sar_zach_trigger_add " << trigger.origVerts[0].x << " " << trigger.origVerts[0].y << " " << trigger.origVerts[0].z
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

CON_COMMAND(sar_zach_trigger_add, "sar_zach_trigger_add <A.x> <A.y> <A.z> <B.x> <B.y> <B.z> <angle> <id> - add a trigger with the specified position, angle and ID.\n")
{
    if (args.ArgC() != 9) {
        return console->Print(sar_zach_trigger_add.ThisPtr()->m_pszHelpString);
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

CON_COMMAND(sar_zach_trigger_place, "sar_zach_trigger_place <id> - place a trigger with the given ID at the position being aimed at.\n")
{
    if (args.ArgC() != 2) {
        return console->Print(sar_zach_trigger_place.ThisPtr()->m_pszHelpString);
    }

    char* end;
    int id = std::strtol(args[1], &end, 10);
    if (*end != 0 || end == args[1]) {
        // ID argument is not a number
        return console->Print(sar_zach_trigger_place.ThisPtr()->m_pszHelpString);
    }

    CGameTrace tr;
    bool hit = engine->TraceFromCamera(65535.0f, tr);
    if (!hit) {
        return console->Print("You aimed at the void.\n");
    }

    if (!sar_zach_show_triggers.GetBool()) {
        console->Print("sar_zach_show_triggers set to 1 !\n");
        sar_zach_show_triggers.SetValue(1);
    }

    if (!zachStats->isFirstPlaced) {
        zachStats->A = tr.endpos;
        zachStats->isFirstPlaced = true;
    } else {
        zachStats->AddTrigger(zachStats->A, tr.endpos, 0.0f, id);
        zachStats->isFirstPlaced = false;
    }
}

CON_COMMAND(sar_zach_trigger_rotate, "sar_zach_trigger_rotate <id> <angle> - changes the rotation of a trigger to the given angle, in degrees.\n")
{
    if (args.ArgC() != 3) {
        return console->Print(sar_zach_trigger_rotate.ThisPtr()->m_pszHelpString);
    }

    char* end;

    int id = std::strtol(args[1], &end, 10);
    if (*end != 0 || end == args[1]) {
        // ID argument is not a number
        return console->Print(sar_zach_trigger_place.ThisPtr()->m_pszHelpString);
    }

    int angle = std::strtol(args[2], &end, 10);
    if (*end != 0 || end == args[2]) {
        // ID argument is not a number
        return console->Print(sar_zach_trigger_place.ThisPtr()->m_pszHelpString);
    }

    auto trigger = zachStats->GetTriggerByID(id);
    if (trigger == nullptr) {
        return console->Print("No such trigger!\n");
    }

    trigger->SetRotation(angle);
}

CON_COMMAND(sar_zach_trigger_delete, "sar_zach_trigger_delete <id> - deletes the trigger with the given ID.\n")
{
    if (args.ArgC() != 2) {
        return console->Print(sar_zach_trigger_delete.ThisPtr()->m_pszHelpString);
    }

    char* end;
    int id = std::strtol(args[1], &end, 10);
    if (*end != 0 || end == args[1]) {
        // ID argument is not a number
        return console->Print(sar_zach_trigger_place.ThisPtr()->m_pszHelpString);
    }

    zachStats->DeleteTrigger(id);
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
