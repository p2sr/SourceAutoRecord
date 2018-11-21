#include "StatsExport.hpp"

#include "Features/Session.hpp"
#include "Features/Tas/TasTools.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Utils/Math.hpp"
#include "Utils/SDK.hpp"

#include <cmath>
#include <fstream>

StatsExport* statsExport;

StatsExport::StatsExport()
{
    this->m_last_tick = 0;
    this->isrecording = false;
}
void StatsExport::Record()
{
    int current_tick = engine->GetSessionTick();
    if (current_tick == this->m_last_tick) {
        return;
    }
    this->velocity.push_back(client->GetLocalVelocity().Length2D());
    this->acceleration.push_back(tasTools->GetAcceleration().z);
    this->m_last_tick = current_tick;
}
void StatsExport::Export(std::string filename)
{
    std::ofstream file;
    file.open(filename + ".csv");
    if (file.bad()) {
        console->Warning("Failed to create file !\n");
        return;
    }

    file << "Frames;Velocity;Acceleration\n";
    int frames = this->acceleration.size();
    for (int i = 0; i < frames; i++) {
        file << std::to_string(i + 1) + ";" + std::to_string(this->velocity[i]) + ";" + std::to_string(this->acceleration[i]) << std::endl;
    }
    file.close();
    statsExport->acceleration.clear();
    statsExport->velocity.clear();
    console->Print("Exporting %s.csv done !\n", filename.c_str());
}
//Commands
CON_COMMAND(sar_stats_start_recording, "sar_stats_start_recording : start to record the velocity each frames.\n")
{
    statsExport->acceleration.clear();
    statsExport->velocity.clear();
    statsExport->isrecording = true;
    console->Print("Velocity recording started.\n");
}
CON_COMMAND(sar_stats_stop_recording, "sar_stats_stop_recording : stop the recording.\n")
{
    statsExport->isrecording = false;
    console->Print("Velocity recording stoped.\n");
}
CON_COMMAND(sar_stats_export_recording, "sar_stats_export_recording <File_Name> : export the stats to the file.\n")
{
    if (args.ArgC() < 2) {
        console->Warning("File name missing : sar_stats_export_recording <File_Name>.\n");
        return;
    }
    if (statsExport->acceleration.size() == 0) {
        console->Warning("Nothing to export !\n");
        return;
    }
    statsExport->Export(args[1]);
}