#include "Event.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Utils/TwitchConnection.hpp"
#include "Variable.hpp"

Variable sar_twitch_chat_enabled("sar_twitch_chat_enabled", "0", "Enables Twitch chat integration. 2 enables spectator command !spec\n");
Variable sar_twitch_chat_channel("sar_twitch_chat_channel", "", "The Twitch channel to connect to.\n");
Variable sar_twitch_chat_color("sar_twitch_chat_color", "255 255 255", "The color of the Twitch chat messages.\n");

TwitchConnection twitchConnection;

ON_EVENT(PRE_TICK) {
    if (!sar_twitch_chat_enabled.GetBool() || strlen(sar_twitch_chat_channel.GetString()) == 0) {
        if (twitchConnection.IsConnected()) {
            twitchConnection.Disconnect();
        }
        return;
    }
    std::string channel = sar_twitch_chat_channel.GetString();
    if (twitchConnection.GetChannel().compare(channel) != 0) {
        twitchConnection.JoinChannel(channel);
    }
    if (!twitchConnection.IsConnected()) {
        twitchConnection.JoinChannel(channel);
    }
    auto twitchMsgs = twitchConnection.FetchNewMessages();
    for (auto msg : twitchMsgs) {
        if (sar_twitch_chat_enabled.GetInt() == 2) {
            std::string message = msg.message;
            message.erase(remove_if(message.begin(), message.end(), [](char c) {
                return c == '"' || c == '\n' || c == '\r' || c == '\0';
            }), message.end());
            std::string author = msg.username;
            author.erase(remove_if(author.begin(), author.end(), [](char c) {
                return c == '"' || c == '\n' || c == '\r' || c == '\0';
            }), author.end());
            if (Utils::StartsWith(message.c_str(), "!spec ")) {
                std::string specName = message.substr(6);
                if (specName.length() > 0) {
                    specName = std::string("ghost_spec_pov \"") + specName.c_str() + "\"\n";
                    engine->ExecuteCommand(specName.c_str(), true);
                }
            } else if (message == "!orbit") {
                engine->ExecuteCommand("ghost_spec_thirdperson 1; +left", true);
                Scheduler::InHostTicks(30*60, []() {
                    engine->ExecuteCommand("ghost_spec_thirdperson 0; -left", true);
                });
            } else if (message == "!unorbit") {
                engine->ExecuteCommand("ghost_spec_thirdperson 0; -left", true);
            } else if (message == "!reconn") {
                engine->ExecuteCommand("ghost_disconnect; ghost_spec_connect dip.portal2.sr", true);
            } else if (Utils::StartsWith(message.c_str(), "!cmd ") && Utils::ICompare(author, sar_twitch_chat_channel.GetString())) {
                std::string command = message.substr(5);
                if (command.length() > 0) {
                    command = command + "\n";
                    engine->ExecuteCommand(command.c_str(), true);
                }
            } else if (Utils::StartsWith(message.c_str(), "!sr ") || Utils::StartsWith(message.c_str(), "!songs ") || Utils::ICompare(author, "nightbot")) {
                // Ignore
            } else if (Utils::ICompare(author, sar_twitch_chat_channel.GetString())) {
                std::string out = std::string("ghost_message \"") + message + "\"\n";
                engine->ExecuteCommand(out.c_str(), true);
            } else {
                std::string out = std::string("ghost_message \"(TTV) ") + author + ": " + message + "\"\n";
                engine->ExecuteCommand(out.c_str(), true);
            }
        } else {
            std::string message = msg.username + ": " + msg.message;
            Color color = Utils::GetColor(sar_twitch_chat_color.GetString()).value_or(Color(255, 255, 255));
            client->Chat(color, message.c_str());
        }
    }
}

ON_EVENT(SAR_UNLOAD) {
    if (twitchConnection.IsConnected()) {
        twitchConnection.Disconnect();
    }
}
