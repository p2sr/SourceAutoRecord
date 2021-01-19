#include "SeamshotFind.hpp"

#include "Features/Session.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Command.hpp"
#include "Variable.hpp"

Variable sar_seamshot_finder("sar_seamshot_finder", "0", 0, 1, "Enables or disables seamshot finder overlay.\n");
Variable sar_seamshot_helper("sar_seamshot_helper", "0", 0, 1, "Enables or disables seamshot helper overlay.\n");

SeamshotFind* seamshotFind;

SeamshotFind::SeamshotFind()
{
    this->hasLoaded = true;
}

CGameTrace TracePortalShot(const Vector& start, const Vector& dir, float length)
{
    CGameTrace tr;

    Vector finalDir = Vector(dir.x, dir.y, dir.z).Normalize() * length;

    Ray_t ray;
    ray.m_IsRay = true;
    ray.m_IsSwept = true;
    ray.m_Start = VectorAligned(start.x, start.y, start.z);
    ray.m_Delta = VectorAligned(finalDir.x, finalDir.y, finalDir.z);
    ray.m_StartOffset = VectorAligned();
    ray.m_Extents = VectorAligned();

    CTraceFilterSimple filter;
    filter.SetPassEntity(server->GetPlayer(GET_SLOT() + 1));

    engine->TraceRay(engine->engineTrace->ThisPtr(), ray, MASK_SHOT_PORTAL, &filter, &tr);

    //hack
    if (ray.m_Start.y == start.y) {
        return tr;
    }
    return tr;
}

void SeamshotFind::DrawLines()
{
    if (sv_cheats.GetBool() && (sar_seamshot_finder.GetBool() || sar_seamshot_helper.GetBool())) {
        void* player = server->GetPlayer(GET_SLOT() + 1);

        if (player == nullptr || (int)player == -1)
            return;

        Vector camPos = server->GetAbsOrigin(player) + server->GetViewOffset(player);

        QAngle angle = engine->GetAngles(GET_SLOT());

        float X = DEG2RAD(angle.x), Y = DEG2RAD(angle.y);
        auto cosX = std::cos(X), cosY = std::cos(Y);
        auto sinX = std::sin(X), sinY = std::sin(Y);

        Vector dir(cosY * cosX, sinY * cosX, -sinX);

        CGameTrace tr = TracePortalShot(camPos, dir, 65536.0);

        //hit something
        if (tr.plane.normal.Length() > 0.9) {

            //look for an edge point
            Vector checkDirs[4];

            //a vector lying on a plane
            Vector upVector = Vector(0, 0, 1);
            if (tr.plane.normal.z * tr.plane.normal.z == 1) {
                upVector = Vector(1, 0, 0);
            }
            checkDirs[0] = tr.plane.normal.Cross(upVector).Normalize();
            //a vector crossing the previous one
            checkDirs[1] = tr.plane.normal.Cross(checkDirs[0]).Normalize();
            //the rest is the inverse of other vectors to get 4 vectors in all directions
            checkDirs[2] = checkDirs[0] * -1;
            checkDirs[3] = checkDirs[1] * -1;

#ifdef _WIN32
#define ADD_LINE_OVERLAY(...) engine->AddLineOverlay(__VA_ARGS__)
#else
#define ADD_LINE_OVERLAY(...) engine->AddLineOverlay(nullptr, __VA_ARGS__)
#endif

            if (sar_seamshot_finder.GetBool()) {
                CGameTrace edgeTr;
                edgeTr.fraction = 999;
                for (int i = 0; i < 4; i++) {
                    CGameTrace newEdgeTr = TracePortalShot(tr.endpos, checkDirs[i], 10);
                    if (newEdgeTr.fraction < edgeTr.fraction) {
                        edgeTr = newEdgeTr;
                    }
                }
                if (edgeTr.fraction < 1) {
                    //two quick tests for a seamshot.
                    Vector test1v = tr.plane.normal + edgeTr.plane.normal * 2;
                    Vector test2v = tr.plane.normal * 2 + edgeTr.plane.normal;

                    Vector test1o = tr.plane.normal * 0.001;
                    Vector test2o = edgeTr.plane.normal * 0.001;

                    CGameTrace test1tr = TracePortalShot(edgeTr.endpos + test1v - test1o, test1v * -1, 4);
                    CGameTrace test2tr = TracePortalShot(edgeTr.endpos + test2v - test2o, test2v * -1, 4);

                    bool test1 = test1tr.plane.normal.Length() == 0; //no surface were hit
                    bool test2 = test2tr.plane.normal.Length() == 0;
                    bool seamshot = test1 || test2;

                    int uiScale = 10;

                    //calculating an edge vector for drawing
                    Vector edge = edgeTr.plane.normal.Cross(tr.plane.normal).Normalize();
                    ADD_LINE_OVERLAY(edgeTr.endpos - edge * uiScale, edgeTr.endpos + edge * uiScale, seamshot ? 0 : 255, seamshot ? 255 : 0, 0, true, 0.06);

                    ADD_LINE_OVERLAY(edgeTr.endpos, edgeTr.endpos + edgeTr.plane.normal * uiScale, test1 ? 0 : 255, test1 ? 255 : 0, 0, true, 0.06);
                    ADD_LINE_OVERLAY(edgeTr.endpos, edgeTr.endpos + tr.plane.normal * uiScale, test2 ? 0 : 255, test2 ? 255 : 0, 0, true, 0.06);

                    if (seamshot) {
                        Vector midPoint = edgeTr.endpos + edgeTr.plane.normal * (uiScale / 2.0) + tr.plane.normal * (uiScale / 2.0);
                        ADD_LINE_OVERLAY(midPoint, edgeTr.endpos + edgeTr.plane.normal * uiScale, test1 ? 0 : 255, test1 ? 255 : 0, 0, true, 0.06);
                        ADD_LINE_OVERLAY(midPoint, edgeTr.endpos + tr.plane.normal * uiScale, test2 ? 0 : 255, test2 ? 255 : 0, 0, true, 0.06);
                    }

                    //ADD_LINE_OVERLAY(edgeTr.endpos + test1v + test1o, edgeTr.endpos + test1v * -2 + test1o, 0, 0, 255, true, 0.06);
                    //ADD_LINE_OVERLAY(edgeTr.endpos + test2v + test2o, edgeTr.endpos + test2v * -2 + test2o, 0, 0, 255, true, 0.06);
                }
            }

            if (sar_seamshot_helper.GetBool()) {
                int uiScale = 5;
                ADD_LINE_OVERLAY(tr.endpos, tr.endpos + tr.plane.normal * uiScale, 0, 0, 255, true, 0.06);
                ADD_LINE_OVERLAY(tr.endpos - checkDirs[0] * uiScale, tr.endpos + checkDirs[0] * uiScale, 0, 255, 0, true, 0.06);
                ADD_LINE_OVERLAY(tr.endpos - checkDirs[1] * uiScale, tr.endpos + checkDirs[1] * uiScale, 255, 0, 0, true, 0.06);
            }
        }
    }
}
