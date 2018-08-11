#pragma once
#include <cstdlib>

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"

#include "Features/Routing.hpp"

#include "Command.hpp"

using namespace Routing;

CON_COMMAND(sar_trace_a, "Saves location A for tracing.\n")
{
    Tracer::Start(Client::GetAbsOrigin());
    console->Print("Saved location A for tracing!\n");
}

CON_COMMAND(sar_trace_b, "Saves location B for tracing.\n")
{
    Tracer::Stop(Client::GetAbsOrigin());
    console->Print("Saved location B for tracing!\n");
}

CON_COMMAND(sar_trace_result, "Prints tracing result.\n")
{
    auto xyz = Tracer::GetDifferences();
    console->Print("A: %.3f/%.3f/%.3f\n", Tracer::Source.x, Tracer::Source.y, Tracer::Source.z);
    console->Print("B: %.3f/%.3f/%.3f\n", Tracer::Destination.x, Tracer::Destination.y, Tracer::Destination.z);
    console->Print("dX: %.3f\n", std::get<0>(xyz));
    console->Print("dY: %.3f\n", std::get<1>(xyz));
    console->Print("dZ: %.3f\n", std::get<2>(xyz));
    console->Print("dXY: %.3f\n", Tracer::GetResult(Tracer::ResultType::VEC2));
    console->Print("dXYZ: %.3f\n", Tracer::GetResult(Tracer::ResultType::VEC3));
}

CON_COMMAND(sar_trace_reset, "Resets tracer.\n")
{
    Tracer::Reset();
}

/* CON_COMMAND(sar_fuzz, "Experimental.\n")
{
    static std::vector<ConCommandBase*> cmdCache;

    auto cmd = tier1->m_pConCommandList;
    if (cmdCache.size() == 0) {
        do {
            if (!std::strncmp(cmd->m_pszName, "sar_", 4)) {
                continue;
            }
            cmdCache.push_back(cmd);
        } while (cmd = cmd->m_pNext);
    }

    auto amount = 1;
    auto times = 1;
    if (args.ArgC() >= 2) {
        amount = atoi(args[1]);
        if (args.ArgC() == 3) {
            times = atoi(args[2]);
        }
    }

    do {
        auto rand = std::rand() % cmdCache.size();
        auto cmd = cmdCache.at(rand);
        auto i = times;
        console->Print("Executing: %s\n", cmd->m_pszName);

        auto IsCommand = reinterpret_cast<bool (*)(void*)>(Memory::VMT(cmd, Offsets::IsCommand));
        if (!IsCommand(cmd)) {
            auto cvar = reinterpret_cast<ConVar*>(cmd);
            do {
                auto args = std::rand() % 11;
                console->Msg("Args: %i\n", args);
                switch (args) {
                case 0:
                    Engine::ClientCommand("%s %s", cvar->m_pszName, cvar->m_pszDefaultValue);
                    break;
                case 1:
                    Engine::ClientCommand("%s %s", cvar->m_pszName, "SAR");
                    break;
                case 2:
                    Engine::ClientCommand("%s %i", cvar->m_pszName, cvar->m_nValue + 1);
                    break;
                case 3:
                    Engine::ClientCommand("%s %i", cvar->m_pszName, -1);
                    break;
                case 4:
                    Engine::ClientCommand("%s %i", cvar->m_pszName, 999999999);
                    break;
                case 5:
                    Engine::ClientCommand("%s %s", cvar->m_pszName, "~\"/\\{");
                    break;
                case 6:
                    Engine::ClientCommand("%s %i", cvar->m_pszName, -999999999);
                    break;
                case 7:
                    Engine::ClientCommand("%s %i", cvar->m_pszName, cvar->m_fMinVal);
                    break;
                case 8:
                    Engine::ClientCommand("%s %i", cvar->m_pszName, cvar->m_fMinVal - 1);
                    break;
                case 9:
                    Engine::ClientCommand("%s %i", cvar->m_pszName, cvar->m_fMaxVal);
                    break;
                case 10:
                    Engine::ClientCommand("%s %i", cvar->m_pszName, cvar->m_fMaxVal + 1);
                    break;
                }
            } while (--i);
        } else {
            do {
                auto args = std::rand() % 6;
                console->Msg("Args: %i\n", args);
                switch (args) {
                case 0:
                    Engine::ClientCommand("%s", cmd->m_pszName);
                    break;
                case 1:
                    Engine::ClientCommand("%s %s", cmd->m_pszName, "SAR");
                    break;
                case 2:
                    Engine::ClientCommand("%s %i", cmd->m_pszName, 0);
                    break;
                case 3:
                    Engine::ClientCommand("%s %i", cmd->m_pszName, -1);
                    break;
                case 4:
                    Engine::ClientCommand("%s %i", cmd->m_pszName, 999999999);
                    break;
                case 5:
                    Engine::ClientCommand("%s %s", cmd->m_pszName, "~\"/\\{");
                    break;
                }
            } while (--i);
        }
    } while (--amount);
}
 */