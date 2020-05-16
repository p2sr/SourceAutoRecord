#pragma once
#include "Command.hpp"
#include "Features/Demo/GhostEntity.hpp"
#include "Utils/SDK.hpp"

#include "SFML/Network.hpp"

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

enum class HEADER {
    NONE,
    PING,
    CONNECT,
    DISCONNECT,
    STOP_SERVER,
    MAP_CHANGE,
    HEART_BEAT,
    MESSAGE,
    UPDATE
};

class NetworkManager {

private:
    sf::TcpSocket tcpSocket;
    sf::UdpSocket udpSocket;
    sf::SocketSelector selector;
    sf::IpAddress serverIP;
    unsigned short int serverPort;
    sf::Uint32 ID;

    std::vector<GhostEntity> ghostPool;

    std::thread networkThread;
    std::condition_variable waitForRunning;

public:
    std::atomic<bool> isConnected;
    std::atomic<bool> runThread;
    std::string name;

public:
    NetworkManager();

    void Connect(sf::IpAddress ip, unsigned short int port);
    void Disconnect();
    void StopServer();
    void RunNetwork();

    void SendPlayerData();
    void ReceiveUDPUpdates(std::vector<sf::Packet>& buffer);
    void TreatUDP(std::vector<sf::Packet>& buffer);
    void TreatTCP(sf::Packet& packet);
};

extern NetworkManager networkManager;

extern Command ghost_connect;
extern Command ghost_disconnect;
extern Command ghost_message;
extern Command ghost_ping;