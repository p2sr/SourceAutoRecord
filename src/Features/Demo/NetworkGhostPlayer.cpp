#include "NetworkGhostPlayer.hpp"
#include "DemoGhostPlayer.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Speedrun/SpeedrunTimer.hpp"

#include <queue>
#include <functional>

static std::queue<std::function<void ()>> g_scheduledEvents;

//DataGhost

sf::Packet& operator>>(sf::Packet& packet, QAngle& angle)
{
    return packet >> angle.x >> angle.y >> angle.z;
}

sf::Packet& operator<<(sf::Packet& packet, const QAngle& angle)
{
    return packet << angle.x << angle.y << angle.z;
}

sf::Packet& operator>>(sf::Packet& packet, Vector& vec)
{
    return packet >> vec.x >> vec.y >> vec.z;
}

sf::Packet& operator<<(sf::Packet& packet, const Vector& vec)
{
    return packet << vec.x << vec.y << vec.z;
}

sf::Packet& operator>>(sf::Packet& packet, DataGhost& dataGhost)
{
    return packet >> dataGhost.position >> dataGhost.view_angle;
}

sf::Packet& operator<<(sf::Packet& packet, const DataGhost& dataGhost)
{
    return packet << dataGhost.position << dataGhost.view_angle;
}

//HEADER

sf::Packet& operator>>(sf::Packet& packet, HEADER& header)
{
    sf::Uint8 tmp;
    packet >> tmp;
    header = static_cast<HEADER>(tmp);
    return packet;
}

sf::Packet& operator<<(sf::Packet& packet, const HEADER& header)
{
    return packet << static_cast<sf::Uint8>(header);
}

Variable ghost_TCP_only("ghost_TCP_only", "0", "Lathil's special command :).\n");
Variable ghost_update_rate("ghost_update_rate", "50", 1, "Adjust the update rate. For people with lathil's internet.\n");
Variable ghost_show_difference("ghost_show_difference", "0", "Display time difference between players after they load a map.\n");

std::mutex mutex;

NetworkManager networkManager;

NetworkManager::NetworkManager()
    : serverIP("localhost")
    , serverPort(53000)
    , name("")
    , isCountdownReady(false)
    , modelName("models/props/food_can/food_can_open.mdl")
{
}

void NetworkManager::Connect(sf::IpAddress ip, unsigned short int port)
{
    if (this->tcpSocket.connect(ip, port, sf::seconds(5))) {
        client->Chat(TextColor::GREEN, "Timeout reached! Cannot connect to the server at %s:%d.\n", ip.toString().c_str(), port);
        return;
    }

    if (this->udpSocket.bind(sf::Socket::AnyPort) != sf::Socket::Done) {
        client->Chat(TextColor::GREEN, "Cannot connect to the server at %s:%d.\n", ip.toString().c_str(), port);
        return;
    }

    this->udpSocket.setBlocking(false);
    this->tcpSocket.setBlocking(true);

    this->serverIP = ip;
    this->serverPort = port;

    sf::Packet connection_packet;
    connection_packet << HEADER::CONNECT << this->udpSocket.getLocalPort() << this->name.c_str() << DataGhost{ { 0, 0, 0 }, { 0, 0, 0 } } << this->modelName.c_str() << engine->m_szLevelName << ghost_TCP_only.GetBool();
    this->tcpSocket.send(connection_packet);

    {
        sf::SocketSelector tcpSelector;
        tcpSelector.add(this->tcpSocket);

        sf::Packet confirm_connection;
        if (tcpSelector.wait(sf::seconds(30))) {
            if (this->tcpSocket.receive(confirm_connection) != sf::Socket::Done) {
                client->Chat(TextColor::GREEN, "Error\n");
            }
        } else {
            client->Chat(TextColor::GREEN, "Timeout reached! Can't connect to the server %s:%d.\n", ip.toString().c_str(), port);
            return;
        }

        //Get our ID
        confirm_connection >> this->ID;

        //Add every player connected to the ghostPool
        sf::Uint32 nb_players;
        confirm_connection >> nb_players;
        for (sf::Uint32 i = 0; i < nb_players; ++i) {
            sf::Uint32 ID;
            std::string name;
            DataGhost data;
            std::string model_name;
            std::string current_map;
            confirm_connection >> ID >> name >> data >> model_name >> current_map;

            auto ghost = std::make_shared<GhostEntity>(ID, name, data, current_map);
            ghost->modelName = model_name;
            this->ghostPoolLock.lock();
            this->ghostPool.push_back(ghost);
            this->ghostPoolLock.unlock();
        }
        this->UpdateGhostsSameMap();
        if (engine->isRunning()) {
            this->SpawnAllGhosts();
        }
        client->Chat(TextColor::GREEN, "Successfully connected to the server!\n%d other players connected\n", nb_players);
    } //End of the scope. Will kill the Selector

    this->isConnected = true;
    this->runThread = true;
    this->waitForRunning.notify_one();
    this->networkThread = std::thread(&NetworkManager::RunNetwork, this);
    this->networkThread.detach();
}

void NetworkManager::Disconnect()
{
    if (this->isConnected) {
        this->isConnected = false;
        this->waitForRunning.notify_one();
        this->ghostPoolLock.lock();
        this->ghostPool.clear();
        this->ghostPoolLock.unlock();

        sf::Packet packet;
        packet << HEADER::DISCONNECT << this->ID;
        this->tcpSocket.send(packet);

        this->selector.clear();
        this->tcpSocket.disconnect();
        this->udpSocket.unbind();

        g_scheduledEvents.push([=]() {
            client->Chat(TextColor::GREEN, "You have been disconnected!");
        });
    }
}

void NetworkManager::StopServer()
{
    this->Disconnect();
}

void NetworkManager::PauseNetwork()
{
    this->runThread = false;
}

void NetworkManager::ResumeNetwork()
{
    this->runThread = true;
    this->waitForRunning.notify_one();
}

void NetworkManager::RunNetwork()
{
    this->selector.add(this->tcpSocket);
    this->selector.add(this->udpSocket);

    while (this->isConnected) {
        {
            std::unique_lock<std::mutex> lck(mutex);
            this->waitForRunning.wait(lck, [this] { return this->runThread.load(); });
        }

        auto now = NOW_STEADY();

        if (now > this->lastUpdateTime + std::chrono::milliseconds(ghost_update_rate.GetInt())) {
            // It's been one update rate interval - send our data again
            // if we need to

            if (engine->isRunning() && !engine->IsGamePaused()) {
                this->SendPlayerData();
            }

            this->lastUpdateTime = now;
        }

        if (this->selector.wait(sf::milliseconds(ghost_update_rate.GetInt()))) {
            if (this->selector.isReady(this->udpSocket)) { //UDP
                std::vector<sf::Packet> buffer;
                this->ReceiveUDPUpdates(buffer);
                for (auto& packet : buffer) {
                    this->Treat(packet);
                }
            }

            if (this->selector.isReady(this->tcpSocket)) { //TCP
                sf::Packet packet;
                sf::Socket::Status status;
                status = this->tcpSocket.receive(packet);
                if (status != sf::Socket::Done) {
                    if (status == sf::Socket::Disconnected) { //If connection with the server lost (crash for e.g.)
                        this->Disconnect();
                        break;
                    }
                    continue;
                }
                this->Treat(packet);
            }
        }
    }
}

void NetworkManager::SendPlayerData()
{
    sf::Packet packet;
    packet << HEADER::UPDATE << this->ID;
    auto player = client->GetPlayer(GET_SLOT() + 1);
    if (player) {
        packet << DataGhost{ client->GetAbsOrigin(player), client->GetAbsAngles(player) };
    } else {
        packet << DataGhost{ { 0, 0, 0 }, { 0, 0, 0 } };
    }

    if (!ghost_TCP_only.GetBool()) {
        this->udpSocket.send(packet, this->serverIP, this->serverPort);
    } else {
        this->tcpSocket.send(packet);
    }
}

void NetworkManager::NotifyMapChange()
{
    sf::Packet packet;
    packet << HEADER::MAP_CHANGE << this->ID << engine->m_szLevelName;
    this->tcpSocket.send(packet);
}

void NetworkManager::NotifySpeedrunFinished()
{
    auto total = speedrun->GetTotal();
    auto ipt = speedrun->GetIntervalPerTick();

    sf::Packet packet;
    packet << HEADER::SPEEDRUN_FINISH << this->ID << SpeedrunTimer::Format(total * ipt).c_str();
    this->tcpSocket.send(packet);
}

void NetworkManager::SendMessageToAll(std::string msg)
{
    sf::Packet packet;
    packet << HEADER::MESSAGE << this->ID << msg.c_str();
    this->tcpSocket.send(packet);
    client->Chat(TextColor::LIGHT_GREEN, "%s: %s", this->name.c_str(), msg.c_str());
}

void NetworkManager::SendPing()
{
    sf::Packet packet;
    packet << HEADER::PING << this->ID;
    this->tcpSocket.send(packet);
    this->pingClock.restart();
}

void NetworkManager::ReceiveUDPUpdates(std::vector<sf::Packet>& buffer)
{
    sf::Socket::Status status;
    do {
        sf::Packet packet;
        sf::IpAddress ip;
        unsigned short int port;
        status = this->udpSocket.receive(packet, ip, port);
        if (status == sf::Socket::Done) {
            buffer.push_back(packet);
        }
    } while (status == sf::Socket::Done);
}

void NetworkManager::Treat(sf::Packet& packet)
{
    HEADER header;
    sf::Uint32 ID;
    packet >> header >> ID;

    switch (header) {
    case HEADER::NONE:
        break;
    case HEADER::PING: {
        auto ping = this->pingClock.getElapsedTime();
        g_scheduledEvents.push([=]() {
            client->Chat(TextColor::GREEN, "Ping: %d ms", ping.asMilliseconds());
        });
        break;
    }
    case HEADER::CONNECT: {
        std::string name;
        DataGhost data;
        std::string model_name;
        std::string current_map;
        packet >> name >> data >> model_name >> current_map;
        auto ghost = std::make_shared<GhostEntity>(ID, name, data, current_map);
        ghost->modelName = model_name;

        this->ghostPoolLock.lock();
        this->ghostPool.push_back(ghost);
        this->ghostPoolLock.unlock();

        g_scheduledEvents.push([=]() {
            client->Chat(TextColor::GREEN, "%s has connected in %s!", name.c_str(), current_map.c_str());

            this->UpdateGhostsSameMap();
            if (ghost->sameMap && engine->isRunning()) {
                ghost->Spawn();
            }
        });

        break;
    }
    case HEADER::DISCONNECT: {
        this->ghostPoolLock.lock();
        int toErase = -1;
        for (int i = 0; i < this->ghostPool.size(); ++i) {
            if (this->ghostPool[i]->ID == ID) {
                g_scheduledEvents.push([=]() {
                    client->Chat(TextColor::GREEN, "%s has disconnected!", this->ghostPool[i]->name.c_str());
                    this->ghostPool[i]->DeleteGhost();
                });
                this->ghostPool[i]->isDestroyed = true;
                toErase = i;
                break;
            }
        }
        if (toErase != -1) this->ghostPool.erase(this->ghostPool.begin() + toErase);
        this->ghostPoolLock.unlock();
        break;
    }
    case HEADER::STOP_SERVER:
        this->StopServer();
        break;
    case HEADER::MAP_CHANGE: {
        auto ghost = this->GetGhostByID(ID);
        if (ghost) {
            std::string map;
            packet >> map;
            ghost->currentMap = map;

            g_scheduledEvents.push([=]() {
                if (ghost->isDestroyed) return; // FIXME: this probably works in practice, but it isn't entirely thread-safe

                this->UpdateGhostsSameMap();
                if (ghost_show_advancement.GetBool()) {
                    client->Chat(TextColor::GREEN, "%s is now on %s", ghost->name.c_str(), ghost->currentMap.c_str());
                }

                if (ghost->sameMap) {
                    ghost->Spawn();
                } else {
                    ghost->DeleteGhost();
                }

                if (ghost_sync.GetBool()) {
                    if (this->AreAllGhostsAheadOrSameMap()) {
                        engine->SendToCommandBuffer("unpause", 40);
                    }
                }
            });
        }
        break;
    }
    case HEADER::HEART_BEAT: {
        sf::Uint32 token;
        packet >> token;
        sf::Packet response;
        response << HEADER::HEART_BEAT << this->ID << token;
        this->tcpSocket.send(response);
        break;
    }
    case HEADER::MESSAGE: {
        auto ghost = this->GetGhostByID(ID);
        if (ghost) {
            std::string message;
            packet >> message;
            g_scheduledEvents.push([=]() {
                client->Chat(TextColor::LIGHT_GREEN, "%s: %s", ghost->name.c_str(), message.c_str());
            });
        }
        break;
    }
    case HEADER::COUNTDOWN: {
        sf::Uint8 step;
        packet >> step;
        if (step == 0) { //Countdown setup
            std::string preCommands;
            std::string postCommands;
            sf::Uint32 duration;
            packet >> duration >> preCommands >> postCommands;

            this->SetupCountdown(preCommands, postCommands, duration);

            sf::Packet confirm_packet;
            confirm_packet << HEADER::COUNTDOWN << this->ID << sf::Uint8(1);
            this->tcpSocket.send(confirm_packet);
        } else if (step == 1) { //Exec
            this->StartCountdown();
        }
        break;
    }
    case HEADER::SPEEDRUN_FINISH: {
        std::string timer;
        packet >> timer;
        auto ghost = this->GetGhostByID(ID);
        if (ghost) {
            if (ghost_show_advancement.GetBool()) {
                g_scheduledEvents.push([=]() {
                    client->Chat(TextColor::GREEN, "%s has finished in %s", ghost->name.c_str(), timer.c_str());
                });
            }
        }
        break;
    }
    case HEADER::MODEL_CHANGE: {
        std::string modelName;
        packet >> modelName;
        auto ghost = this->GetGhostByID(ID);
        if (ghost) {
            ghost->modelName = modelName;
            g_scheduledEvents.push([=]() {
                if (ghost->isDestroyed) return; // FIXME: this probably works in practice, but it isn't entirely thread-safe
                if (ghost->sameMap && engine->isRunning()) {
                    ghost->DeleteGhost();
                    ghost->Spawn();
                }
            });
        }
        break;
    }
    case HEADER::UPDATE: {
        DataGhost data;
        packet >> data;
        auto ghost = this->GetGhostByID(ID);
        if (ghost) {
            ghost->SetData(data.position, data.view_angle, true);
        }
        break;
    }
    default:
        break;
    }
}

void NetworkManager::UpdateGhostsPosition()
{
    this->ghostPoolLock.lock();
    for (auto ghost : this->ghostPool) {
        if (ghost->sameMap) {
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(NOW_STEADY() - ghost->lastUpdate).count();
            ghost->Lerp(((float)time / (ghost->loopTime)));
        }
    }
    this->ghostPoolLock.unlock();
}

std::shared_ptr<GhostEntity> NetworkManager::GetGhostByID(sf::Uint32 ID)
{
    this->ghostPoolLock.lock();
    for (auto ghost : this->ghostPool) {
        if (ghost->ID == ID) {
            this->ghostPoolLock.unlock();
            return ghost;
        }
    }
    this->ghostPoolLock.unlock();
    return nullptr;
}

void NetworkManager::UpdateGhostsSameMap()
{
    int mapIdx = engine->GetMapIndex(engine->m_szLevelName);
    this->ghostPoolLock.lock();
    for (auto ghost : this->ghostPool) {
        ghost->sameMap = ghost->currentMap == engine->m_szLevelName;
        if (mapIdx == -1) ghost->isAhead = false; // Fallback - unknown map
        else ghost->isAhead = engine->GetMapIndex(ghost->currentMap) > mapIdx;
    }
    this->ghostPoolLock.unlock();
}

void NetworkManager::UpdateModel(const std::string modelName)
{
    this->modelName = modelName;
    if (this->isConnected) {
        sf::Packet packet;
        packet << HEADER::MODEL_CHANGE << this->ID << this->modelName.c_str();
        this->tcpSocket.send(packet);
    }
}

bool NetworkManager::AreAllGhostsAheadOrSameMap()
{
    this->ghostPoolLock.lock();
    for (auto ghost : this->ghostPool) {
        if (!ghost->isAhead && !ghost->sameMap) {
            this->ghostPoolLock.unlock();
            return false;
        }
    }
    this->ghostPoolLock.unlock();

    return true;
}

void NetworkManager::SpawnAllGhosts()
{
    this->ghostPoolLock.lock();
    for (auto ghost : this->ghostPool) {
        if (ghost->sameMap) {
            ghost->Spawn();
        }
    }
    this->ghostPoolLock.unlock();
}

void NetworkManager::DeleteAllGhosts()
{
    this->ghostPoolLock.lock();
    for (auto ghost : this->ghostPool) {
        ghost->DeleteGhost();
    }
    this->ghostPoolLock.unlock();
}

void NetworkManager::SetupCountdown(std::string preCommands, std::string postCommands, sf::Uint32 duration)
{
    std::string pre = "\"" + preCommands + "\"";
    std::string post = "\"" + postCommands + "\"";
    g_scheduledEvents.push([=]() {
        engine->ExecuteCommand(pre.c_str());
    });
    this->postCountdownCommands = postCommands;
    this->countdownStep = duration;
    this->timeLeft = NOW_STEADY();
}

void NetworkManager::StartCountdown()
{
    auto ping = std::chrono::duration_cast<std::chrono::milliseconds>(NOW_STEADY() - this->timeLeft);
    this->timeLeft = NOW_STEADY() + std::chrono::seconds(3) - ping;
    this->isCountdownReady = true;
}

void NetworkManager::UpdateCountdown()
{
    auto now = NOW_STEADY();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - this->timeLeft).count() >= 1000) {
        if (this->countdownStep == 0) {
            client->Chat(TextColor::GREEN, "0! GO!");
            if (!this->postCountdownCommands.empty()) {
                engine->ExecuteCommand(this->postCountdownCommands.c_str());
            }
            this->isCountdownReady = false;
        } else {
            client->Chat(TextColor::LIGHT_GREEN, "%d...", this->countdownStep);
        }
        this->countdownStep--;
        this->timeLeft = now;
    }
}

void NetworkManager::DrawNames(HudContext* ctx)
{
    auto player = client->GetPlayer(GET_SLOT() + 1);
    if (player) {
        this->ghostPoolLock.lock();
        for (int i = 0; i < this->ghostPool.size(); ++i) {
            if (this->ghostPool[i]->sameMap) {
                Vector screenPos;
                engine->PointToScreen(this->ghostPool[i]->data.position, screenPos);
                ctx->DrawElementOnScreen(i, screenPos.x, screenPos.y - ghost_text_offset.GetInt() - ghost_height.GetInt(), this->ghostPool[i]->name.c_str());
            }
        }
        this->ghostPoolLock.unlock();
    }
}

void NetworkManager::DispatchQueuedEvents()
{
    while (!g_scheduledEvents.empty()) {
        g_scheduledEvents.front()(); // Dispatch event
        g_scheduledEvents.pop();
    }
}

// Commands

CON_COMMAND(ghost_connect, "Connect to the server : <ip address> <port> :\n"
                           "ex: 'localhost 53000' - '127.0.0.1 53000' - 89.10.20.20 53000'.\n")
{
    if (args.ArgC() < 2) {
        return console->Print(ghost_connect.ThisPtr()->m_pszHelpString);
    }

    if (networkManager.name == "") {
        return console->Print("Please change your name with \"ghost_name\" before connecting to the server.\n");
    }

    networkManager.Connect(args[1], std::atoi(args[2]));
}

CON_COMMAND(ghost_disconnect, "Disconnect.\n")
{
    networkManager.Disconnect();
}

CON_COMMAND(ghost_name, "Change your online name.\n")
{
    if (networkManager.isConnected) {
        return console->Print("Cannot change name while connected to a server.\n");
    }

    networkManager.name = args[1];
}

CON_COMMAND(ghost_message, "Send message to other players.\n")
{
    if (args.ArgC() < 2) {
        return console->Print(ghost_message.ThisPtr()->m_pszHelpString);
    }

    std::string msg = args[1];
    for (int i = 2; i < args.ArgC(); ++i) {
        msg += " " + std::string(args[i]);
    }

    networkManager.SendMessageToAll(msg);
}

CON_COMMAND(ghost_ping, "Pong!\n")
{
    networkManager.SendPing();
}
