#include "NetworkGhostPlayer.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

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


Variable ghost_sync("ghost_sync", 0, "When loading a new level, pauses the game until other players load it.\n");



std::mutex mutex;

NetworkManager networkManager;

NetworkManager::NetworkManager()
    : serverIP("localhost")
    , serverPort(53000)
    , name("me")
    , isCountdownReady(false)
{
}

void NetworkManager::Connect(sf::IpAddress ip, unsigned short int port)
{
    if (this->tcpSocket.connect(ip, port, sf::seconds(5))) {
        client->Chat(TextColor::GREEN, "Timeout reached ! Can't connect to the server %s:%d !\n", ip.toString().c_str(), port);
        return;
    }

    if (this->udpSocket.bind(sf::Socket::AnyPort) != sf::Socket::Done) {
        client->Chat(TextColor::GREEN, "Can't connect to the server %s:%d !\n", ip.toString().c_str(), port);
        return;
    }

    this->udpSocket.setBlocking(false);
    this->tcpSocket.setBlocking(true);

    this->serverIP = ip;
    this->serverPort = port;

    client->Chat(TextColor::GREEN, "%s : %d", this->tcpSocket.getRemoteAddress().toString().c_str(), this->udpSocket.getLocalPort());

    sf::Packet connection_packet;
    connection_packet << HEADER::CONNECT << this->udpSocket.getLocalPort() << this->name.c_str() << DataGhost{ { 0, 0, 0 }, { 0, 0, 0 } } << engine->m_szLevelName;
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
            client->Chat(TextColor::GREEN, "Timeout reached ! Can't connect to the server %s:%d !\n", ip.toString().c_str(), port);
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
            std::string current_map;
            confirm_connection >> ID >> name >> data >> current_map;

            GhostEntity ghost(ID, name, data, current_map);
            this->ghostPool.push_back(ghost);
        }
        this->UpdateGhostsSameMap();
        client->Chat(TextColor::GREEN, "Successfully connected to the server !\n%d player connected\n", nb_players);
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
        this->ghostPool.clear();

        sf::Packet packet;
        packet << HEADER::DISCONNECT << this->ID;
        this->tcpSocket.send(packet);

        this->selector.clear();
        this->tcpSocket.disconnect();
        this->udpSocket.unbind();

        client->Chat(TextColor::GREEN, "You have been disconnected !");
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

        if (engine->isRunning()) {
            this->SendPlayerData();
        }

        if (this->selector.wait(sf::milliseconds(50))) {
            if (this->selector.isReady(this->udpSocket)) { //UDP
                std::vector<sf::Packet> buffer;
                this->ReceiveUDPUpdates(buffer);
                this->TreatUDP(buffer);
            } else if (this->selector.isReady(this->tcpSocket)) { //TCP
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
                this->TreatTCP(packet);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
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

    this->udpSocket.send(packet, this->serverIP, this->serverPort);
}

void NetworkManager::NotifyMapChange()
{
    sf::Packet packet;
    packet << HEADER::MAP_CHANGE << this->ID << engine->m_szLevelName;
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

void NetworkManager::TreatUDP(std::vector<sf::Packet>& buffer)
{
    for (auto& packet : buffer) {
        HEADER header;
        packet >> header;
        sf::Uint32 ID;
        packet >> ID;
        for (auto& g : this->ghostPool) {
            if (g.ID == ID) {
                DataGhost data;
                packet >> data;
                g.SetData(data.position, data.view_angle);
            }
        }
    }
}

void NetworkManager::TreatTCP(sf::Packet& packet)
{
    HEADER header;
    sf::Uint32 ID;
    packet >> header >> ID;

    switch (header) {
    case HEADER::NONE:
        break;
    case HEADER::PING: {
        auto ping = this->pingClock.getElapsedTime();
        client->Chat(TextColor::GREEN, "Ping ! (%d ms)", ping.asMilliseconds());
        break;
    }
    case HEADER::CONNECT: {
        std::string name;
        DataGhost data;
        std::string current_map;
        packet >> name >> data >> current_map;
        GhostEntity ghost(ID, name, data, current_map);
        this->ghostPool.push_back(ghost);
        client->Chat(TextColor::GREEN, "%s has connected !", name.c_str());
        this->UpdateGhostsSameMap();
        break;
    }
    case HEADER::DISCONNECT: {
        for (int i = 0; i < this->ghostPool.size(); ++i) {
            if (this->ghostPool[i].ID == ID) {
                client->Chat(TextColor::GREEN, "%s has disconnected !", this->ghostPool[i].name.c_str());
                this->ghostPool.erase(this->ghostPool.begin() + i);
            }
        }
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
            this->UpdateGhostsSameMap();
            client->Chat(TextColor::GREEN, "%s is now on %s", ghost->name.c_str(), ghost->currentMap.c_str());
            if (ghost_sync.GetBool()) {
                if (this->AreGhostsOnSameMap() && engine->IsGamePaused()) {
                    engine->ExecuteCommand("unpause");
                }
            }
        }
        break;
    }
    case HEADER::HEART_BEAT:
        break;
    case HEADER::MESSAGE: {
        auto ghost = this->GetGhostByID(ID);
        if (ghost) {
            std::string message;
            packet >> message;
            client->Chat(TextColor::LIGHT_GREEN, "%s: %s", ghost->name.c_str(), message.c_str());
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
            confirm_packet << HEADER::COUNTDOWN << sf::Uint8(1);
            this->tcpSocket.send(confirm_packet);
        } else if (step == 1) { //Exec
            this->StartCountdown();
        }
        break;
    }
    default:
        break;
    }
}

void NetworkManager::UpdateGhostsPosition()
{
    for (auto& ghost : this->ghostPool) {
        if (ghost.sameMap) {
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(NOW() - ghost.lastUpdate).count();
            ghost.Lerp(((float)time / (ghost.loopTime)));
        }
    }
}

GhostEntity* NetworkManager::GetGhostByID(sf::Uint32 ID)
{
    for (auto& ghost : this->ghostPool) {
        if (ghost.ID == ID) {
            return &ghost;
        }
    }
    return nullptr;
}

void NetworkManager::UpdateGhostsSameMap()
{
    for (auto& ghost : this->ghostPool) {
        ghost.sameMap = ghost.currentMap == engine->m_szLevelName;
    }
}

bool NetworkManager::AreGhostsOnSameMap()
{
    for (auto& ghost : this->ghostPool) {
        if (!ghost.sameMap) {
            return false;
        }
    }

    return true;
}

void NetworkManager::SetupCountdown(std::string preCommands, std::string postCommands, sf::Uint32 duration)
{
    engine->ExecuteCommand(preCommands.c_str());
    this->postCountdownCommands = postCommands;
    this->countdownStep = duration;
    this->timeLeft = NOW();
}

void NetworkManager::StartCountdown()
{
    auto ping = std::chrono::duration_cast<std::chrono::milliseconds>(NOW() - this->timeLeft);
    this->timeLeft = NOW() + std::chrono::seconds(3) - ping;
    this->isCountdownReady = true;
}

void NetworkManager::UpdateCountdown()
{
    auto now = NOW();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - this->timeLeft).count() >= 1000) {
        if (this->countdownStep == 0) {
            client->Chat(TextColor::GREEN, "0 ! GO !");
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


// Commands

CON_COMMAND(ghost_connect, "Connect to server.\n")
{
    if (args.ArgC() < 2) {
        return console->Print(ghost_connect.ThisPtr()->m_pszHelpString);
    }

    networkManager.Connect(args[1], std::atoi(args[2]));
    //networkManager.Connect("192.168.1.64", 53000);
}

CON_COMMAND(ghost_disconnect, "Disconnect.\n")
{
    networkManager.Disconnect();
}

CON_COMMAND(lmao, "test")
{
    networkManager.name = args[1];
}

CON_COMMAND(ghost_message, "Send message")
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

CON_COMMAND(ghost_ping, "Pong !")
{
    networkManager.SendPing();
}