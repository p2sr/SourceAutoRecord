#pragma once

#include <string>
#include <thread>
#include <vector>
#include <functional>

class NetworkConnection {
private:
	int socketID = 0;
	std::string ip;
	int port;
	bool dataReady = false;
	bool connected = false;
	std::thread connThread;

	int bufflen = 0;
	char sockbuff[16384];
public:
	NetworkConnection(std::string ip, int port);
	void ChangeAddress(std::string ip, int port);
	bool Connect();
	void Disconnect();
	bool TryProcessData(std::function<void(char *, int)> func);
	void SendData(char *data, int size);
	void SendData(std::string data);
	bool IsConnected() { return connected; }
};