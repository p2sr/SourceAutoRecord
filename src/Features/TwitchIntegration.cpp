#include "Event.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
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
        std::string message = msg.message;
        std::string author = msg.username;
        if (message.length() == 0 || author.length() == 0)
            continue;
        
        if (sar_twitch_chat_enabled.GetInt() == 2) {
            if (Utils::StartsWith(message.c_str(), "!spec ")) {
                std::string specName = message.substr(6);
                specName.erase(remove_if(specName.begin(), specName.end(), [](char c) {
                    return c == '"' || c == '\n' || c == '\r' || c == '\0';
                }), specName.end());
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
            } else if (Utils::StartsWith(message.c_str(), "!cmd ")) {
                if (Utils::ICompare(author, "thisamj") || Utils::ICompare(author, sar_twitch_chat_channel.GetString())) {
                    // obama medal for obama
                    // i promise i can be trusted :P
                    std::string command = message.substr(5);
                    if (command.length() > 0) {
                        engine->ExecuteCommand(command.c_str(), true);
                    }
                } else {
                    engine->ExecuteCommand("sar_toast_create warning Nuh uh.", true);
                }
            } else if (Utils::StartsWith(message.c_str(), "!") || Utils::ICompare(author, "nightbot") || Utils::ICompare(author, "streamelements")) {
                // Ignore bots and bot commands
            } else if (Utils::ICompare(author, sar_twitch_chat_channel.GetString())) {
                networkManager.SendMessageToAll(message);
            } else {
                networkManager.SendMessageToAll("(TTV) " + author + ": " + message);
            }
        } else {
            if (!(Utils::StartsWith(message.c_str(), "!") || Utils::ICompare(author, "nightbot") || Utils::ICompare(author, "streamelements"))) {
                std::string message = msg.username + ": " + msg.message;
                Color color = Utils::GetColor(sar_twitch_chat_color.GetString()).value_or(Color(255, 255, 255));
                client->Chat(color, message.c_str());
            }
        }
    }
}

ON_EVENT(SAR_UNLOAD) {
    if (twitchConnection.IsConnected()) {
        twitchConnection.Disconnect();
    }
}
