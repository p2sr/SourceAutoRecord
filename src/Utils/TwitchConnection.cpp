#include "TwitchConnection.hpp"

#include "Modules/Console.hpp"

#include <cstring>
#include <sstream>

namespace {
bool IsEmojiCodePoint(uint32_t codePoint) {
	if (codePoint == 0x200D || codePoint == 0x20E3 || codePoint == 0xFE0F) {
		return true;
	}

	if (codePoint >= 0x1F000 && codePoint <= 0x1FAFF) {
		return true;
	}
	if (codePoint >= 0x1F1E6 && codePoint <= 0x1F1FF) {
		return true;
	}
	if (codePoint >= 0x1F300 && codePoint <= 0x1FAD6) {
		return true;
	}
	if (codePoint >= 0x1F600 && codePoint <= 0x1F64F) {
		return true;
	}
	if (codePoint >= 0x1F680 && codePoint <= 0x1F6FF) {
		return true;
	}
	if (codePoint >= 0x2600 && codePoint <= 0x26FF) {
		return true;
	}
	if (codePoint >= 0x2700 && codePoint <= 0x27BF) {
		return true;
	}
	if (codePoint >= 0xFE00 && codePoint <= 0xFE0F) {
		return true;
	}

	return false;
}

std::string StripEmojiFromUtf8(const std::string &input) {
	std::string output;
	output.reserve(input.size());

	for (size_t i = 0; i < input.size();) {
		const unsigned char lead = static_cast<unsigned char>(input[i]);

		if (lead < 0x80) {
			output.push_back(input[i]);
			++i;
			continue;
		}

		size_t length = 1;
		if ((lead & 0xE0) == 0xC0) {
			length = 2;
		} else if ((lead & 0xF0) == 0xE0) {
			length = 3;
		} else if ((lead & 0xF8) == 0xF0) {
			length = 4;
		} else {
			output.push_back(input[i]);
			++i;
			continue;
		}

		if (i + length > input.size()) {
			output.append(input, i, input.size() - i);
			break;
		}

		uint32_t codePoint = 0;
		switch (length) {
		case 2:
			codePoint = ((lead & 0x1F) << 6) | (static_cast<unsigned char>(input[i + 1]) & 0x3F);
			break;
		case 3:
			codePoint = ((lead & 0x0F) << 12) |
				((static_cast<unsigned char>(input[i + 1]) & 0x3F) << 6) |
				(static_cast<unsigned char>(input[i + 2]) & 0x3F);
			break;
		case 4:
			codePoint = ((lead & 0x07) << 18) |
				((static_cast<unsigned char>(input[i + 1]) & 0x3F) << 12) |
				((static_cast<unsigned char>(input[i + 2]) & 0x3F) << 6) |
				(static_cast<unsigned char>(input[i + 3]) & 0x3F);
			break;
		default:
			output.append(input, i, length);
			i += length;
			continue;
		}

		if (IsEmojiCodePoint(codePoint)) {
			output.append("?");
			i += length;
			continue;
		}

		output.append(input, i, length);
		i += length;
	}

	return output;
}
}

TwitchConnection::TwitchConnection() 
 : NetworkConnection("irc.chat.twitch.tv", 6667) {

}

void TwitchConnection::JoinChannel(std::string name) {
	channel = name;

	Connect();

	SendData("CAP REQ :twitch.tv/tags twitch.tv/commands\nNICK justinfan8\nJOIN #" + channel + "\n");
}

std::vector<TwitchConnection::Message> TwitchConnection::FetchNewMessages() {

	std::vector<TwitchConnection::Message> messages;

	TryProcessData([&](char *sockbuff, int messageLen) {
		std::istringstream data(std::string(sockbuff, messageLen));
		std::string line;
		while (std::getline(data, line)) {
			if (line.empty()) continue;
			// console->Print("Twitch: '%s'\n", line.c_str());
			if (line[0] == 'P') { // PING :tmi.twitch.tv
				SendData("PONG :tmi.twitch.tv\n");
			} else if (std::strstr(line.c_str(), "PRIVMSG")) {
				auto displaynameStart = line.find("display-name=");
				if (displaynameStart == std::string::npos) continue;
				std::string displayname(line.begin() + displaynameStart + 13, line.begin() + line.find(';', displaynameStart + 13));

				auto usernameStart = line.find(":") + 1;
				if (usernameStart == std::string::npos) continue;
				std::string username(line.begin() + usernameStart, line.begin() + line.find("!", usernameStart));

				auto colorStart = line.find("color=");
				std::optional<Color> color;
				if (colorStart != std::string::npos) {
					std::string colorStr(line.begin() + colorStart + 6, line.begin() + line.find(';', colorStart + 6));
					color = Utils::GetColor(colorStr.c_str());
				}

				std::string msg(line.begin() + line.find(":", line.find("PRIVMSG")) + 1, line.end());
				msg = StripEmojiFromUtf8(Utils::trimR(msg));
				// repeated messages have 0xcd8f at the end
				if (msg.size() >= 2 && (unsigned char)(msg[msg.size() - 2]) == 0xcd && (unsigned char)(msg[msg.size() - 1]) == 0x8f) {
					msg = Utils::trimR(msg.substr(0, msg.size() - 2));
				}

				messages.push_back({displayname, username, color.value_or(Color(0, 0, 0)), msg});
			}
		}
	});

	return messages;
}
