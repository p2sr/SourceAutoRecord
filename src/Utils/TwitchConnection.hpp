#include "NetworkConnection.hpp"

#pragma once
class TwitchConnection : protected NetworkConnection{
public:
	struct Message{
		std::string username;
		std::string message;
	};
private:
	std::string channel;

public:
	TwitchConnection();
	void JoinChannel(std::string name);
	std::string GetChannel() { return channel; }
	std::vector<Message> FetchNewMessages();

	using NetworkConnection::Connect;
	using NetworkConnection::Disconnect;
	using NetworkConnection::IsConnected;
	using NetworkConnection::SendData;
};

