#include "EngineDemoPlayer.hpp"

#include "Features/Demo/Demo.hpp"
#include "Features/Demo/DemoParser.hpp"
#include "Features/Camera.hpp"
#include "Features/Renderer.hpp"

#include "Console.hpp"
#include "Engine.hpp"
#include "Server.hpp"
#include "Client.hpp"

#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"
#include "Checksum.hpp"

#include <filesystem>

#ifdef SAR_MODERATOR_BUILD
Variable sar_demo_cheat_info("sar_demo_cheat_info", "0", 0, 1, "Display anticheat info in demo playback.\n");
#endif

REDECL(EngineDemoPlayer::StartPlayback);
REDECL(EngineDemoPlayer::stopdemo_callback);

int EngineDemoPlayer::GetTick()
{
    return this->GetPlaybackTick(this->s_ClientDemoPlayer->ThisPtr());
}
bool EngineDemoPlayer::IsPlaying()
{
    return this->IsPlayingBack(this->s_ClientDemoPlayer->ThisPtr());
}

void EngineDemoPlayer::ClearDemoQueue()
{
    engine->demoplayer->demoQueue.clear();
    engine->demoplayer->demoQueueSize = 0;
    engine->demoplayer->currentDemoID = -1;
}

std::string EngineDemoPlayer::GetLevelName()
{
    return this->levelName;
}

void EngineDemoPlayer::CustomDemoData(char* data, size_t length)
{
    if (data[0] == 0x03) { // Entity input data
        char *targetname = data + 1;
        size_t targetnameLen = strlen(targetname);
        char *classname = data + 2 + targetnameLen;
        size_t classnameLen = strlen(classname);
        char *inputname = data + 3 + targetnameLen + classnameLen;
        size_t inputnameLen = strlen(inputname);
        char *parameter = data + 4 + targetnameLen + classnameLen + inputnameLen;
        // TODO: send to mtrigger/custom category stuff
        return;
    }

#ifdef SAR_MODERATOR_BUILD
    if (sar_demo_cheat_info.GetBool()) {
        if (data[0] == 0xFF) {
            // Checksum data should be at tick -1 (and hence never run
            // through this callback), so this suggestes a tampered demo
            client->Chat(TextColor::ORANGE, "Unexpected checksum data! Has the demo been tampered with?");
            return;
        } else if (data[0] == 0x01 && length == 5) {
            // Timescale cheat warning
            client->Chat(TextColor::ORANGE, "CHEAT: timescale %.2f", *(float*)(data+1));
            return;
        } else if (data[0] == 0x02) {
            // Initial variable value
            const char* name = data + 1;
            size_t nameLen = strlen(name);
            if (nameLen < length - 2) {
                const char* value = data + nameLen + 2;
                size_t valueLen = strlen(value);
                if (nameLen + valueLen + 3 == length) {
                    client->Chat(TextColor::LIGHT_GREEN, "INITIAL: %s = %s", name, value);
                    return;
                }
            }
        }
        // Unknown or invalid data
        client->Chat(TextColor::ORANGE, "Malformed custom demo info! Has the demo been tampered with?");
    }
#endif
}

DETOUR_COMMAND(EngineDemoPlayer::stopdemo)
{
    engine->demoplayer->ClearDemoQueue();
    EngineDemoPlayer::stopdemo_callback(args);
}

// CDemoRecorder::StartPlayback
DETOUR(EngineDemoPlayer::StartPlayback, const char* filename, bool bAsTimeDemo)
{
#ifdef SAR_MODERATOR_BUILD
    if (sar_demo_cheat_info.GetBool()) {
        auto filepath = std::string(engine->GetGameDirectory()) + "/" + filename;
        auto res = VerifyDemoChecksum(filepath.c_str());
        switch (res.first) {
        case VERIFY_BAD_DEMO:
            // Normal chat rather than queue as we probably aren't loading
            // into the demo (it seems invalid)
            client->Chat(TextColor::ORANGE, "Could not read checksum for demo!");
            break;

        case VERIFY_NO_CHECKSUM:
            client->QueueChat(TextColor::ORANGE, "No checksum found! Was the demo recorded without SAR?");
            break;

        case VERIFY_INVALID_CHECKSUM:
            client->QueueChat(TextColor::ORANGE, "Demo checksum invalid! Has the demo been tampered with?");
            client->QueueChat(TextColor::LIGHT_GREEN, "SAR checksum: %.8X", res.second);
            break;

        case VERIFY_VALID_CHECKSUM:
            client->QueueChat(TextColor::GREEN, "Demo checksum verified");
            client->QueueChat(TextColor::LIGHT_GREEN, "SAR checksum: %.8X", res.second);
            break;
        }
    }
#endif

    auto result = EngineDemoPlayer::StartPlayback(thisptr, filename, bAsTimeDemo);

    if (result) {
        DemoParser parser;
        Demo demo;
        auto dir = std::string(engine->GetGameDirectory()) + std::string("/")
            + std::string(engine->demoplayer->DemoName);
        if (parser.Parse(dir, &demo)) {
            parser.Adjust(&demo);
            console->Print("Client:   %s\n", demo.clientName);
            console->Print("Map:      %s\n", demo.mapName);
            console->Print("Ticks:    %i\n", demo.playbackTicks);
            console->Print("Time:     %.3f\n", demo.playbackTime);
            console->Print("Tickrate: %.3f\n", demo.Tickrate());
            engine->demoplayer->levelName = demo.mapName;
            Renderer::demoStart = demo.firstPositivePacketTick;
            Renderer::segmentEndTick = demo.segmentTicks;

#ifdef SAR_MODERATOR_BUILD
            if (sar_demo_cheat_info.GetBool()) {
                client->QueueChat(TextColor::LIGHT_GREEN, "Average of %.2fTPS", (float)demo.playbackTicks / demo.playbackTime);
            }
#endif
        } else {
            console->Print("Could not parse \"%s\"!\n", engine->demoplayer->DemoName);
        }
    }

    camera->RequestTimeOffsetRefresh();

    Renderer::isDemoLoading = true;

    return result;
}

bool EngineDemoPlayer::Init()
{
    auto disconnect = engine->cl->Original(Offsets::Disconnect);
    auto demoplayer = Memory::DerefDeref<void*>(disconnect + Offsets::demoplayer);
    if (this->s_ClientDemoPlayer = Interface::Create(demoplayer)) {
        this->s_ClientDemoPlayer->Hook(EngineDemoPlayer::StartPlayback_Hook, EngineDemoPlayer::StartPlayback, Offsets::StartPlayback);

        this->GetPlaybackTick = s_ClientDemoPlayer->Original<_GetPlaybackTick>(Offsets::GetPlaybackTick);
        this->IsPlayingBack = s_ClientDemoPlayer->Original<_IsPlayingBack>(Offsets::IsPlayingBack);
        this->DemoName = reinterpret_cast<char*>((uintptr_t)demoplayer + Offsets::m_szFileName);
    }

    Command::Hook("stopdemo", EngineDemoPlayer::stopdemo_callback_hook, EngineDemoPlayer::stopdemo_callback);

    this->currentDemoID = -1;
    this->demoQueueSize = 0;

    return this->hasLoaded = this->s_ClientDemoPlayer;
}
void EngineDemoPlayer::Shutdown()
{
    Interface::Delete(this->s_ClientDemoPlayer);
    Command::Unhook("stopdemo", EngineDemoPlayer::stopdemo_callback);
}

// Commands

CON_COMMAND_AUTOCOMPLETEFILE(sar_startdemos, "Improved version of startdemos. 'sar_startdemos <demoname>' Use 'stopdemo' to stop playing demos.\n",
    0, 0, dem)
{
    // Always print a useful message for the user if not used correctly
    if (args.ArgC() <= 1) {
        return console->Print(sar_startdemos.ThisPtr()->m_pszHelpString);
    }

    engine->demoplayer->demoQueue.clear();

    std::string name = args[1];

    if (name.length() > 4) {
        if (name.substr(name.length() - 4, 4) == ".dem")
            name.resize(name.length() - 4);
    }

    auto dir = engine->GetGameDirectory() + std::string("/");
    int counter = 2;

    Demo demo;
    DemoParser parser;
    bool ok = parser.Parse(dir + name, &demo);

    if (!ok) {
        return console->Print("Could not parse \"%s\"!\n", engine->GetGameDirectory() + std::string("/") + args[1]);
    }

    engine->demoplayer->demoQueue.push_back(name);

    while (ok) {
        auto tmp_dir = dir + name + "_" + std::to_string(counter);
        ok = parser.Parse(tmp_dir, &demo);
        if (ok) {
            engine->demoplayer->demoQueue.push_back(name + "_" + std::to_string(counter));
        }
        ++counter;
    }

    engine->demoplayer->demoQueueSize = engine->demoplayer->demoQueue.size();
    engine->demoplayer->currentDemoID = 0;

    EngineDemoPlayer::stopdemo_callback(args);

    //Demos are played in Engine::Frame
}
CON_COMMAND(sar_startdemosfolder, "sar_startdemosfolder <folder name>. Plays all the demos in the specified folder by alphabetic order.\n")
{
    if (args.ArgC() < 2) {
        return console->Print(sar_startdemosfolder.ThisPtr()->m_pszHelpString);
    }

    engine->demoplayer->demoQueue.clear();

    auto dir = engine->GetGameDirectory() + std::string("/");
    std::string filepath;
    Demo demo;
    DemoParser parser;

    for (const auto& file : std::filesystem::directory_iterator(dir + args[1])) {
        if (file.path().extension() != ".dem")
            continue;

        filepath = args[1] + std::string("/") + file.path().filename().string();
        console->Print("%s\n", filepath.c_str());
        if (parser.Parse(dir + filepath, &demo)) {
            engine->demoplayer->demoQueue.push_back(filepath);
        }
    }

    std::sort(engine->demoplayer->demoQueue.begin(), engine->demoplayer->demoQueue.end());

    engine->demoplayer->demoQueueSize = engine->demoplayer->demoQueue.size();
    engine->demoplayer->currentDemoID = 0;

    EngineDemoPlayer::stopdemo_callback(args);
}
CON_COMMAND_COMPLETION(sar_skiptodemo, "sar_skiptodemo <demoname>. Skip demos in demo queue to this demo.\n", ({ engine->demoplayer->demoQueue }))
{
    if (args.ArgC() < 2) {
        return console->Print(sar_skiptodemo.ThisPtr()->m_pszHelpString);
    }

    auto it = std::find(engine->demoplayer->demoQueue.begin(), engine->demoplayer->demoQueue.end(), args[1]);
    if (it == engine->demoplayer->demoQueue.end())
        return;

    engine->demoplayer->currentDemoID =  std::distance(engine->demoplayer->demoQueue.begin(), it);

    EngineDemoPlayer::stopdemo_callback(args);
}
CON_COMMAND(sar_nextdemo, "Plays the next demo in demo queue.\n")
{
    if (++engine->demoplayer->currentDemoID >= engine->demoplayer->demoQueueSize) {
        return engine->demoplayer->ClearDemoQueue();
    }

    EngineDemoPlayer::stopdemo_callback(args);
}
