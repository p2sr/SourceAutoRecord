#include "TwitchConnection.hpp"

#include <cstring>

TwitchConnection::TwitchConnection() 
 : NetworkConnection("irc.chat.twitch.tv", 6667) {

}

void TwitchConnection::JoinChannel(std::string name) {
	channel = name;

	Connect();

	SendData("NICK justinfan8\nJOIN #" + channel + "\n");
}

std::vector<TwitchConnection::Message> TwitchConnection::FetchNewMessages() {

	std::vector<TwitchConnection::Message> messages;

	TryProcessData([&](char *sockbuff, int messageLen) {
		char *startChar = sockbuff;

		for (char *c = sockbuff; *c != '\0'; c++) {
			if (*c == '\n') {
				int len = (c - startChar);
				std::string message(startChar, len);

				if (message[0] == 'P') {
					SendData("PONG :tmi.twitch.tv\n");
				} else if (message[0] == ':' && std::strstr(message.c_str(), "PRIVMSG")) {
					std::string nickname(startChar + 1, message.find('!') - 1);
					std::string msg(message.begin() + message.find(':', 1) + 1, message.end());
					messages.push_back({nickname, msg});
				}

				startChar += len + 1;
			}
		}
	});

	return messages;
}