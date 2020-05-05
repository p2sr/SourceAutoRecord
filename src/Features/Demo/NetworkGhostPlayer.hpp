#pragma once
#include "GhostEntity.hpp"

#include "Features/Demo/Demo.hpp"
#include "Features/Feature.hpp"

#include "Command.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"

#include "SFML/Network.hpp"

#include <atomic>
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
    MESSAGE,
    COUNTDOWN,
    UPDATE,
    MODEL_CHANGE,
};

class NetworkGhostPlayer : public Feature {

private:
    bool isConnected;
    sf::SocketSelector selector;
    std::condition_variable waitForPaused;
    bool waitForPlayers;

public:
    sf::IpAddress ip_client;
    std::vector<GhostEntity*> ghostPool;
    std::string name;
    std::string modelName;
    int defaultGhostType;
    sf::IpAddress ip_server;
    unsigned short port_server;
    sf::UdpSocket socket;
    sf::TcpSocket tcpSocket;
    std::thread networkThread;
    std::thread TCPThread;
    std::atomic<bool> runThread;
    std::atomic<bool> pauseThread;
    std::chrono::time_point<std::chrono::steady_clock> start;
    std::chrono::milliseconds tickrate;
    bool isInLevel;
    bool pausedByServer;
    int countdown;
    std::chrono::time_point<std::chrono::steady_clock> startCountdown;
    bool isCountdownReady;
    std::string commandPreCountdown;
    std::string commandPostCountdown;

private:
    void NetworkThink();
    void CheckConnection();
    GhostEntity* SetupGhost(sf::Uint32& ID, std::string name, DataGhost&, std::string&, std::string& modelName);
    void UpdatePlayer();
    void SetupCountdown(sf::Uint32 time);

public:
    NetworkGhostPlayer();

    void ConnectToServer(std::string, unsigned short port);
    void Disconnect();
    void StopServer();
    void Countdown();
    bool IsConnected();

    sf::Socket::Status ReceivePacket(sf::Packet& packet, sf::IpAddress&, int timeout);

    void StartThinking();
    void PauseThinking();

    DataGhost GetPlayerData();
    GhostEntity* GetGhostByID(const sf::Uint32& ID);
    void SetPosAng(sf::Uint32& ID, Vector position, Vector angle);
    void UpdateCurrentMap();
    void UpdateGhostsCurrentMap();
    bool AreGhostsOnSameMap();
    void ClearGhosts();
    void UpdateModel();
};

extern NetworkGhostPlayer* networkGhostPlayer;

extern Variable sar_ghost_sync_maps;
extern Variable sar_ghost_show_progress;
extern Variable sar_ghost_TCP_only;

extern Command sar_ghost_connect_to_server;
extern Command sar_ghost_disconnect;
extern Command sar_ghost_name;
extern Command sar_ghost_tickrate;
extern Command sar_ghost_countdown;
extern Command sar_ghost_message;
