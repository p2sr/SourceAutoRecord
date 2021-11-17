#pragma once
#include "Command.hpp"
#include "Features/Demo/GhostEntity.hpp"
#include "Features/Hud/Hud.hpp"
#include "SFML/Network.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
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
	COUNTDOWN,
	UPDATE,
	SPEEDRUN_FINISH,
	MODEL_CHANGE
};

class NetworkManager {
public:
	sf::TcpSocket tcpSocket;
	sf::UdpSocket udpSocket;
	sf::SocketSelector selector;
	sf::IpAddress serverIP;
	unsigned short int serverPort;
	sf::Uint32 ID;

	std::mutex ghostPoolLock;
	std::vector<std::shared_ptr<GhostEntity>> ghostPool;

	std::thread networkThread;
	std::condition_variable waitForRunning;

	sf::Clock pingClock;
	sf::Clock updateClock;

	std::string postCountdownCommands;
	std::chrono::time_point<std::chrono::steady_clock> timeLeft;
	int countdownStep;
	bool countdownShow;

	std::chrono::time_point<std::chrono::steady_clock> lastUpdateTime;

public:
	std::atomic<bool> isConnected;
	std::atomic<bool> runThread;
	std::string name;
	bool isCountdownReady;
	std::string modelName;

	sf::Uint32 splitTicks = -1;
	sf::Uint32 splitTicksTotal = -1;

	bool disableSyncForLoad = false;

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
	void NotifySpeedrunFinished(const bool CM = false);
	void SendMessageToAll(std::string msg);
	void SendPing();
	void ReceiveUDPUpdates(std::vector<sf::Packet> &buffer);
	void Treat(sf::Packet &packet, bool udp);

	void UpdateGhostsPosition();
	std::shared_ptr<GhostEntity> GetGhostByID(sf::Uint32 ID);
	void UpdateGhostsSameMap();
	void UpdateModel(const std::string modelName);
	bool AreAllGhostsAheadOrSameMap();
	void SpawnAllGhosts();
	void DeleteAllGhosts();

	void SetupCountdown(std::string preCommands, std::string postCommands, sf::Uint32 duration);
	//Need this function to measure the ping in order to start the countdown at the same time
	void StartCountdown();
	//Print the state of the countdown
	void UpdateCountdown();

	void DrawNames(HudContext *ctx);
};

extern NetworkManager networkManager;

extern Variable ghost_TCP_only;
extern Variable ghost_update_rate;
extern Command ghost_connect;
extern Command ghost_disconnect;
extern Command ghost_message;
extern Command ghost_ping;
extern Command ghost_name;
