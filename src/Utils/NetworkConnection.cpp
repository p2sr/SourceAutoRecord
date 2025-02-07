#include "NetworkConnection.hpp"

#include <iostream>

#ifdef _WIN32
#	include <winsock2.h>
#	include <ws2tcpip.h>
#else
#	include <cstring>
#	include <netdb.h>
#	include <sys/socket.h>
#	include <sys/types.h>
#	include <unistd.h>
#	define INVALID_SOCKET -1
#	define SOCKET_ERROR -1
#endif

NetworkConnection::NetworkConnection(std::string ip, int port) {
	ChangeAddress(ip, port);
}

void NetworkConnection::ChangeAddress(std::string ip, int port) {
	if (IsConnected()) Disconnect();
	this->ip = ip;
	this->port = port;
}

bool NetworkConnection::Connect() {
	Disconnect();

#ifdef WIN32
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;
#endif

	if ((socketID = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) return false;

	addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;
	if ((getaddrinfo(ip.c_str(), NULL, &hints, &res)) != 0) return false;
	sockaddr_in addr;
	addr.sin_addr = ((sockaddr_in *)res->ai_addr)->sin_addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	freeaddrinfo(res);

	if (connect(socketID, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) return false;

	connected = true;
	connThread = std::thread([&]() {
		while (IsConnected()) {
			while (dataReady);

			// checking if there is a message to receive
			fd_set fdset{1, {socketID}};
			timeval time{0, 10000};
			int selectResult = select(socketID + 1, &fdset, NULL, NULL, &time);
			if (selectResult < 0) break;
			if (selectResult == 0) continue;

			// receiving a message
			memset(&sockbuff, '\0', sizeof(sockbuff));
			bufflen = recv(socketID, sockbuff, sizeof(sockbuff) - 1, 0);
			if (bufflen <= 0) break;

			dataReady = true;
		}
		connected = false;
	});

	return true;
}

void NetworkConnection::Disconnect() {
	connected = false;
	dataReady = false;

	if (socketID != 0) {
		shutdown(socketID, 2);  // SHUT_RDWR on Linux, SD_BOTH on Windows

		connThread.join();
		#ifdef WIN32
			closesocket(socketID);
			WSACleanup();
		#else
			close(socketID);
		#endif

		socketID = 0;
	}
}

bool NetworkConnection::TryProcessData(std::function<void(char *, int)> func) {
	if (!dataReady) return false;
	func(sockbuff, bufflen);
	dataReady = false;
	return true;
}

void NetworkConnection::SendData(char *data, int size) {
	send(socketID, data, size, 0);
}

void NetworkConnection::SendData(std::string data) {
	SendData((char*)data.c_str(), data.size());
}
