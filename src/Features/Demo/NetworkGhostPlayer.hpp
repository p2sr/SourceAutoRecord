#pragma once
#include "Command.hpp"
#include "Variable.hpp"
#include "Features/Demo/GhostEntity.hpp"
#include "Utils/SDK.hpp"
#include "Features/Hud/Hud.hpp"

#include "SFML/Network.hpp"

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>

enum class HEADER {
    NONE,
    PING,
    CONNECT,
    DISCONNECT,
    STOP_SERVER,
    MAP_CHANGE,
    HEART_BEAT,
    MESSAGE,
    COUNTDOWN,
    UPDATE,
    SPEEDRUN_FINISH,
    MODEL_CHANGE
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

    sf::Clock pingClock;
    sf::Clock updateClock;

    std::string postCountdownCommands;
    std::chrono::time_point<std::chrono::steady_clock> timeLeft;
    int countdownStep;

    std::chrono::time_point<std::chrono::steady_clock> lastUpdateTime;

public:
    std::atomic<bool> isConnected;
    std::atomic<bool> runThread;
    std::string name;
    bool isCountdownReady;
    std::string modelName;

public:
    NetworkManager();

    void Connect(sf::IpAddress ip, unsigned short int port);
    void Disconnect();
    void StopServer();
    void PauseNetwork();
    void ResumeNetwork();
    void RunNetwork();

    void SendPlayerData();
    void NotifyMapChange();
    void NotifySpeedrunFinished();
    void SendMessageToAll(std::string msg);
    void SendPing();
    void ReceiveUDPUpdates(std::vector<sf::Packet>& buffer);
    void TreatUDP(std::vector<sf::Packet>& buffer);
    void TreatTCP(sf::Packet& packet);

    void UpdateGhostsPosition();
    GhostEntity* GetGhostByID(sf::Uint32 ID);
    void UpdateGhostsSameMap();
    void UpdateModel(const std::string modelName);
    bool AreAllGhostsAheadOrSameMap();
    void SpawnAllGhosts();
    void DeleteAllGhosts();
    void DeleteAllGhostModels(const bool newEntity);

    void SetupCountdown(std::string preCommands, std::string postCommands, sf::Uint32 duration);
    //Need this function to mesure the ping in order to start the countdown at the same time
    void StartCountdown();
    //Print the state of the countdown
    void UpdateCountdown();

    void DispatchQueuedEvents();

    void DrawNames(HudContext *ctx);
};

extern NetworkManager networkManager;

extern Variable ghost_TCP_only;
extern Variable ghost_update_rate;
extern Variable ghost_show_difference;
extern Command ghost_connect;
extern Command ghost_disconnect;
extern Command ghost_message;
extern Command ghost_ping;
extern Command ghost_name;
