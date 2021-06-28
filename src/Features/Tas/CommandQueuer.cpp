#include "CommandQueuer.hpp"

#include <algorithm>
#include <array>
#include <string>

#include "Features/Session.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Variable.hpp"

Variable sar_tas_autostart("sar_tas_autostart", "1", "Starts queued commands automatically on first frame after a load.\n");
Variable sar_tas_ss_forceuser("sar_tas_ss_forceuser", "0", "Forces engine to calculate movement for every splitescreen client.\n");

CommandQueuer* cmdQueuer;

CommandQueuer::CommandQueuer()
    : frames()
    , isRunning(false)
    , baseIndex(0)
    , curSplitScreen(0)
    , curDelay(0)
    , numberRegex("([-+]?\\d*(\\d|(\\.\\d))\\d*([eE]([+-]?\\d+))?)")
    , floatRegex("\\[" + numberRegex + ":" + numberRegex + "\\]")
    , intRegex("\\{" + numberRegex + ":" + numberRegex + "\\}")
    , currentFrame(-1)
{
    this->hasLoaded = true;
}
void CommandQueuer::AddFrame(int framesLeft, std::string command, bool relative)
{
    if (relative) {
        framesLeft += this->baseIndex;
    } else {
        this->baseIndex = framesLeft;
    }

    this->frames.push_back(CommandFrame{
        framesLeft + this->curDelay,
        command,
        this->curSplitScreen });
}
void CommandQueuer::AddFrames(int framesLeft, int interval, int lastFrame, std::string command, bool relative)
{
    if (relative) {
        framesLeft += baseIndex;
        lastFrame += baseIndex;
    } else {
        baseIndex = framesLeft;
    }

    for (; framesLeft <= lastFrame; framesLeft += interval) {
        this->frames.push_back(CommandFrame{
            framesLeft + this->curDelay,
            command,
            this->curSplitScreen });
    }
}
void CommandQueuer::CalculateOffset(int framesLeft)
{
    this->baseIndex += framesLeft;
}
void CommandQueuer::SetSplitScreen(int splitScreen)
{
    this->curSplitScreen = splitScreen;
}
void CommandQueuer::Stop()
{
    this->isRunning = false;
}
void CommandQueuer::Reset()
{
    this->Stop();
    this->baseIndex = 0;
    this->frames.clear();
    this->curDelay = 0;
}
void CommandQueuer::Start()
{
    if (!this->frames.empty() && sv_cheats.GetBool()) {
        std::sort(this->frames.begin(), this->frames.end(), [](const auto& a, const auto& b) {
            return a.framesLeft < b.framesLeft;
        });
        this->isRunning = true;
        this->currentFrame = 0;
    }
}
void CommandQueuer::DelayQueueBy(int frames)
{
    this->curDelay = frames;
}
void CommandQueuer::RandomRegex(std::string& input)
{
    for (std::sregex_iterator it = std::sregex_iterator(input.begin(), input.end(), this->floatRegex); it != std::sregex_iterator(); ++it) {
        std::smatch m = *it;
        float rand = Math::RandomNumber(std::stof(m.str(1)), std::stof(m.str(6)));
        input = std::regex_replace(input, this->floatRegex, std::to_string(rand), std::regex_constants::format_first_only);
    }

    for (std::sregex_iterator it = std::sregex_iterator(input.begin(), input.end(), this->intRegex); it != std::sregex_iterator(); ++it) {
        std::smatch m = *it;
        int rand = Math::RandomNumber(std::stoi(m.str(1)), std::stoi(m.str(6)));
        input = std::regex_replace(input, this->intRegex, std::to_string(rand), std::regex_constants::format_first_only);
    }
}

void CommandQueuer::Execute()
{
    //FIX FOR PIKACHU <3

    auto tick = session->GetTick();
    if (this->currentFrame == tick)
        return;

    if (cmdQueuer->isRunning) {
        for (auto&& tas = cmdQueuer->frames.begin(); tas != cmdQueuer->frames.end();) {
            --tas->framesLeft;

            if (tas->framesLeft <= 0) {
                console->DevMsg("[%i] %s\n", session->currentFrame, tas->command.c_str());

                if (engine->GetMaxClients() <= 1) {
                    engine->Cbuf_AddText(tas->splitScreen, tas->command.c_str(), 0);
                } else {
                    auto entity = engine->PEntityOfEntIndex(tas->splitScreen + 1);
                    if (entity && !entity->IsFree() && server->IsPlayer(entity->m_pUnk)) {
                        engine->ClientCommand(nullptr, entity, tas->command.c_str());
                    }
                }

                tas = cmdQueuer->frames.erase(tas);
            } else {
                ++tas;
            }
        }
        currentFrame++;
    }

    ++session->currentFrame;
}

// Commands

CON_COMMAND(sar_tas_frame_at,
    "sar_tas_frame_at <frame> [command_to_execute] - adds command frame to the queue at specified frame\n")
{
    IGNORE_DEMO_PLAYER();

    if (args.ArgC() != 3) {
        return console->Print(sar_tas_frame_at.ThisPtr()->m_pszHelpString);
    }

    std::array<std::string, 2> tab_args = { args[1], args[2] };
    for (int i = 0; i < 2; i++) {
        if (tab_args[i][0] == '[' || tab_args[i][0] == '{' || i == 1) {
            cmdQueuer->RandomRegex(tab_args[i]);
        }
    }

    cmdQueuer->AddFrame(std::atoi(tab_args[0].c_str()), tab_args[1]);
}
CON_COMMAND(sar_tas_frames_at,
    "sar_tas_frames_at <frame> <interval> <last_frame> [command_to_execute] - adds command frame multiple times to the queue at specified frame\n")
{
    IGNORE_DEMO_PLAYER();

    if (args.ArgC() != 5) {
        return console->Print(sar_tas_frames_at.ThisPtr()->m_pszHelpString);
    }

    std::array<std::string, 4> tab_args = { args[1], args[2], args[3], args[4] };
    for (int i = 0; i < 4; i++) {
        if (tab_args[i][0] == '[' || tab_args[i][0] == '{' || i == 3) {
            cmdQueuer->RandomRegex(tab_args[i]);
        }
    }

    cmdQueuer->AddFrames(std::atoi(tab_args[0].c_str()), std::atoi(tab_args[1].c_str()), std::atoi(tab_args[2].c_str()), tab_args[3]);
}
CON_COMMAND(sar_tas_frame_next,
    "sar_tas_frame_next <frames_to_wait> [command_to_execute] - adds command frame to the queue after waiting for specified amount of frames\n")
{
    IGNORE_DEMO_PLAYER();

    if (args.ArgC() != 3) {
        return console->Print(sar_tas_frame_next.ThisPtr()->m_pszHelpString);
    }

    cmdQueuer->CalculateOffset(std::atoi(args[1]));
    cmdQueuer->AddFrame(0, std::string(args[2]), true);
}
CON_COMMAND(sar_tas_frame_after,
    "sar_tas_frame_after <frames_to_wait> [command_to_execute] - adds command frame to the queue after waiting for specified amount of frames\n")
{
    IGNORE_DEMO_PLAYER();

    if (args.ArgC() != 3) {
        return console->Print(sar_tas_frame_after.ThisPtr()->m_pszHelpString);
    }

    std::array<std::string, 2> tab_args = { args[1], args[2] };
    for (int i = 0; i < 2; i++) {
        if (tab_args[i][0] == '[' || tab_args[i][0] == '{' || i == 1) {
            cmdQueuer->RandomRegex(tab_args[i]);
        }
    }

    cmdQueuer->AddFrame(std::atoi(tab_args[0].c_str()), tab_args[1], true);
}
CON_COMMAND(sar_tas_frames_after,
    "sar_tas_frames_after <frames_to_wait> <interval> <length> [command_to_execute] - adds command frame multiple times to the queue after waiting for specified amount of frames\n")
{
    IGNORE_DEMO_PLAYER();

    if (args.ArgC() != 5) {
        return console->Print(sar_tas_frames_after.ThisPtr()->m_pszHelpString);
    }

    std::array<std::string, 4> tab_args = { args[1], args[2], args[3], args[4] };
    for (int i = 0; i < 4; i++) {
        if (tab_args[i][0] == '[' || tab_args[i][0] == '{' || i == 3) {
            cmdQueuer->RandomRegex(tab_args[i]);
        }
    }

    cmdQueuer->AddFrames(std::atoi(tab_args[0].c_str()), std::atoi(tab_args[1].c_str()), std::atoi(tab_args[2].c_str()), tab_args[3], true);
}
CON_COMMAND(sar_tas_frame_offset,
    "sar_tas_frame_offset <frame> - sar_tas_frame_after rely on the last sar_tas_frame_offset\n")
{
    IGNORE_DEMO_PLAYER();

    if (args.ArgC() != 2) {
        return console->Print(sar_tas_frame_offset.ThisPtr()->m_pszHelpString);
    }

    cmdQueuer->CalculateOffset(std::atoi(args[1]));
}
CON_COMMAND(sar_tas_start, "sar_tas_start - starts executing queued commands\n")
{
    IGNORE_DEMO_PLAYER();

    cmdQueuer->Start();
}
CON_COMMAND(sar_tas_reset, "sar_tas_reset - stops executing commands and clears them from the queue\n")
{
    IGNORE_DEMO_PLAYER();

    cmdQueuer->Reset();
}
CON_COMMAND(sar_tas_ss, "sar_tas_ss <index> - select split screen index for command buffer (0 or 1)\n")
{
    IGNORE_DEMO_PLAYER();

    if (args.ArgC() != 2) {
        return console->Print(sar_tas_ss.ThisPtr()->m_pszHelpString);
    }

    auto index = std::atoi(args[1]);
    if (index == 0 || index == 1) {
        cmdQueuer->SetSplitScreen(index);
    } else {
        console->Print("Invalid split screen index!\n");
    }
}
CON_COMMAND(sar_tas_delay, "sar_tas_delay <frames_to_wait> - delays command queue by specified amount of frames\n")
{
    IGNORE_DEMO_PLAYER();

    if (args.ArgC() != 2) {
        return console->Print(sar_tas_delay.ThisPtr()->m_pszHelpString);
    }

    cmdQueuer->DelayQueueBy(std::atoi(args[1]));
}
CON_COMMAND(sar_tas_frame_at_for,
    "sar_tas_frame_at_for <frame> <delay> <first_command> <last_command> - adds two command frames to the queue at specified frame, the last frame will be executed after a delay\n")
{
    IGNORE_DEMO_PLAYER();

    if (args.ArgC() != 5) {
        return console->Print(sar_tas_frame_at_for.ThisPtr()->m_pszHelpString);
    }

    cmdQueuer->AddFrame(std::atoi(args[1]), std::string(args[3]));
    cmdQueuer->AddFrame(std::atoi(args[1]) + std::atoi(args[2]), std::string(args[4]));
}
CON_COMMAND(sar_tas_frame_after_for,
    "sar_tas_frame_after_for <frames_to_wait> <delay> <first_command> <last_command> - adds two command frames to the queue after waiting for specified amount of frames, "
    "the last frame will be executed after a delay\n")
{
    IGNORE_DEMO_PLAYER();

    if (args.ArgC() != 5) {
        return console->Print(sar_tas_frame_after_for.ThisPtr()->m_pszHelpString);
    }

    cmdQueuer->AddFrame(std::atoi(args[1]), std::string(args[3]), true);
    cmdQueuer->AddFrame(std::atoi(args[1]) + std::atoi(args[2]), std::string(args[4]), true);
}

ON_EVENT(SESSION_START)
{
    if (sar_tas_autostart.GetBool()) {
        cmdQueuer->Start();
    }
}
