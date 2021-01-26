#include "ZachStats.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"

#include <cmath>

#ifdef _WIN32
#define PLAT_CALL(fn, ...) fn(__VA_ARGS__)
#else
#define PLAT_CALL(fn, ...) fn(nullptr, __VA_ARGS__)
#endif

ZachStats* zachStats;

ZachStats::ZachStats()
{
    this->GetRects();
}

void ZachStats::UpdateRects()
{
    auto player = client->GetPlayer(GET_SLOT() + 1);
    if (!player)
        return;

    auto pos = client->GetAbsOrigin(player);

    this->CheckRects(pos);
}

void ZachStats::AddRect(Vector& A, Vector& G)
{
    float lengthX = G.x - A.x;
    float lengthY = G.y - A.y;
    float lengthZ = G.z - A.z;

    Vector B{ A.x + lengthX, A.y, A.z };
    Vector C{ A.x + lengthX, A.y, A.z + lengthZ };
    Vector D{ A.x, A.y, A.z + lengthZ };

    Vector E{ A.x, A.y + lengthY, A.z };
    Vector F{ A.x + lengthX, A.y + lengthY, A.z };
    Vector H{ A.x, A.y + lengthY, A.z + lengthZ };

    //"mouse5" = "sar_stats_rect -150 -400 960 -82 -331 1003"

    Rect r{ A, B, C, D, E, F, G, H };

    this->GetRects().push_back(r);
}

std::vector<Rect>& ZachStats::GetRects()
{
    static std::vector<Rect> rects;
    return rects;
}

void ZachStats::CheckRects(Vector& pos)
{
    for (auto& r : this->GetRects()) {

        PLAT_CALL(engine->AddLineOverlay, r.a, r.b, 255, 0, 0, true, 0);
        PLAT_CALL(engine->AddLineOverlay, r.a, r.d, 255, 0, 0, true, 0);
        PLAT_CALL(engine->AddLineOverlay, r.d, r.c, 255, 0, 0, true, 0);
        PLAT_CALL(engine->AddLineOverlay, r.c, r.b, 255, 0, 0, true, 0);

        PLAT_CALL(engine->AddLineOverlay, r.e, r.h, 255, 0, 0, true, 0);
        PLAT_CALL(engine->AddLineOverlay, r.e, r.f, 255, 0, 0, true, 0);
        PLAT_CALL(engine->AddLineOverlay, r.h, r.g, 255, 0, 0, true, 0);
        PLAT_CALL(engine->AddLineOverlay, r.f, r.g, 255, 0, 0, true, 0);

        PLAT_CALL(engine->AddLineOverlay, r.a, r.e, 255, 0, 0, true, 0);
        PLAT_CALL(engine->AddLineOverlay, r.d, r.h, 255, 0, 0, true, 0);
        PLAT_CALL(engine->AddLineOverlay, r.c, r.g, 255, 0, 0, true, 0);
        PLAT_CALL(engine->AddLineOverlay, r.b, r.f, 255, 0, 0, true, 0);

        // Step 1: collide on y. As the box is effectively axis-aligned on
        // this axis, we can just compare the point's y coordinate to the top
        // and bottom of the cuboid

        float yMin = r.a.y, yMax = r.e.y; // A point on the bottom and top respectively
        if (pos.y < yMin || pos.y > yMax) {
            r.show = false;
            continue;
        }

        // Step 2: collide on x and z. We can now ignore the y axis entirely,
        // and instead look at colliding the point {p.x, p.z} with the
        // rectangle defined by the top or bottom face of the cuboid

        // Algorithm stolen from https://stackoverflow.com/a/2752754/13932065

        float bax = r.b.x - r.a.x;
        float baz = r.b.z - r.a.z;
        float dax = r.d.x - r.a.x;
        float daz = r.d.z - r.a.z;

        if ((pos.x - r.a.x) * bax + (pos.z - r.a.z) * baz < 0) {
            console->Print("1: %f\n", (pos.x - r.a.x) * bax + (pos.z - r.a.z) * baz);
            r.show = false;
            continue;
        }
        if ((pos.x - r.b.x) * bax + (pos.z - r.b.z) * baz < 0) {
            console->Print("2: %f\n", (pos.x - r.b.x) * bax + (pos.z - r.b.z) * baz);
            r.show = false;
            continue;
        }
        if ((pos.x - r.a.x) * dax + (pos.z - r.a.z) * daz < 0) {
            console->Print("3: %f\n", (pos.x - r.a.x) * dax + (pos.z - r.a.z) * daz);
            r.show = false;
            continue;
        }
        if ((pos.x - r.d.x) * dax + (pos.z - r.d.z) * daz < 0) {
            console->Print("4: %f\n", (pos.x - r.d.x) * dax + (pos.z - r.d.z) * daz);
            r.show = false;
            continue;
        }

        r.show = true;
        console->Print("Inside\n");
    }
}

CON_COMMAND(sar_stats_rect, "test")
{
    if (args.ArgC() != 7) {
        return console->Print(sar_stats_rect.ThisPtr()->m_pszHelpString);
    }

    Vector A = Vector(std::atof(args[1]), std::atof(args[2]), std::atof(args[3]));
    Vector G = Vector(std::atof(args[4]), std::atof(args[5]), std::atof(args[6]));

    zachStats->AddRect(A, G);
}