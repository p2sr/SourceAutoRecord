#include "NetworkGhostPlayer.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Utils/SDK.hpp"

#include "GhostEntity.hpp"

#include <chrono>

//DataGhost

sf::Packet& operator>>(sf::Packet& packet, QAngle& angle)
{
    return packet >> angle.x >> angle.y >> angle.z;
}

sf::Packet& operator>>(sf::Packet& packet, DataGhost& dataGhost)
{
    return packet >> dataGhost.position >> dataGhost.view_angle;
}

sf::Packet& operator<<(sf::Packet& packet, const QAngle& angle)
{
    return packet << angle.x << angle.y << angle.z;
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

Variable sar_ghost_sync_maps("sar_ghost_sync_maps", "0", "If the player loads a map first, game will pause until other players reach that same map, keeping everyone synchronized.\n");

NetworkGhostPlayer* networkGhostPlayer;

std::mutex mutex;

NetworkGhostPlayer::NetworkGhostPlayer()
    : ip_server("localhost")
    , port_server(53000)
    , name("FrenchSaves10Ticks")
    , defaultGhostType(1)
    , runThread(false)
    , pauseThread(true)
    , isConnected(false)
    , networkThread()
    , TCPThread()
    , ghostPool()
    , tcpSocket()
    , tickrate(30)
    , isInLevel(false)
    , pausedByServer(false)
    , countdown(-1)
    , modelName("models/props/food_can/food_can_open.mdl")
    , isCountdownReady(false)
{
    this->hasLoaded = true;
    this->socket.setBlocking(false);
}

void NetworkGhostPlayer::ConnectToServer(std::string ip, unsigned short port)
{
    if (tcpSocket.connect(ip, port, sf::seconds(5)) != sf::Socket::Done) {
        client->Chat(TextColor::ORANGE, "0 ! GO !", "Timeout reached ! Can't connect to the server %s:%d !\n", ip.c_str(), port);
        return;
    }

    this->socket.bind(sf::Socket::AnyPort);
    this->selector.add(this->socket);

    this->ip_server = ip;
    this->port_server = port;

    sf::Packet connection_packet;
    connection_packet << HEADER::CONNECT << this->socket.getLocalPort() << this->name;
    if (!*engine->m_szLevelName) { //If not in a level
        connection_packet << DataGhost{ { 0, 0, 0 }, { 0, 0, 0 } };
    } else {
        connection_packet << this->GetPlayerData();
    }

    connection_packet << std::string(engine->m_szLevelName) << this->modelName;
    tcpSocket.send(connection_packet);

    sf::SocketSelector tcpSelector;
    tcpSelector.add(tcpSocket);

    sf::Packet confirmation_packet;
    if (tcpSelector.wait(sf::seconds(30))) {
        if (tcpSocket.receive(confirmation_packet) != sf::Socket::Done) {
            client->Chat(TextColor::ORANGE, "Error\n");
            return;
        }
    } else {
        client->Chat(TextColor::ORANGE, "Timeout reached ! Can't connect to the server %s:%d !\n", ip.c_str(), port);
        return;
    }

    //Add every player connected to the ghostPool
    sf::Uint32 nbPlayer;
    confirmation_packet >> nbPlayer;
    for (sf::Uint32 i = 0; i < nbPlayer; ++i) {
        sf::Uint32 ID;
        std::string name;
        DataGhost data;
        std::string currentMap;
        std::string ghostModelName;
        confirmation_packet >> ID >> name >> data >> currentMap >> ghostModelName;

        if (ghostModelName.empty()) { //If other player is using sar_ghost_type 1
            ghostModelName = "models/props/food_can/food_can_open.mdl";
        }

        this->ghostPool.push_back(this->SetupGhost(ID, name, data, currentMap, ghostModelName));
    }
    client->Chat(TextColor::GREEN, "Successfully connected to the server !\n%d player connected\n", nbPlayer);

    this->isConnected = true;
    this->isInLevel = (engine->m_szLevelName && !engine->m_szLevelName[0]) ? false : true;
    this->pauseThread = !this->isInLevel;
    this->StartThinking();
}

void NetworkGhostPlayer::Disconnect()
{
    this->runThread = false;
    this->waitForPaused.notify_one(); //runThread being false will make the thread stopping no matter if pauseThread is true or false

    sf::Packet packet;
    packet << HEADER::DISCONNECT;
    this->tcpSocket.send(packet);

    for (auto& it : this->ghostPool) {
        it->Stop();
    }
    this->ghostPool.clear();
    this->isConnected = false;
    this->selector.clear();
    this->socket.unbind();
    this->tcpSocket.disconnect();
    this->isInLevel = false;
    while (this->networkThread.joinable())
        ; //Check if the thread is dead
}

void NetworkGhostPlayer::StopServer()
{
    this->runThread = false;
    this->waitForPaused.notify_one(); //runThread being false will make the thread stopping no matter if pauseThread is true or false

    client->Chat(TextColor::LIGHT_GREEN, "Server will stop !\n");

    for (auto& it : this->ghostPool) {
        it->Stop();
    }
    this->ghostPool.clear();
    this->isConnected = false;
    this->selector.clear();
    this->socket.unbind();
    this->tcpSocket.disconnect();
    this->isInLevel = false;
}

bool NetworkGhostPlayer::IsConnected()
{
    return this->isConnected;
}

sf::Socket::Status NetworkGhostPlayer::ReceivePacket(sf::Packet& packet, sf::IpAddress& ip, int timeout)
{
    unsigned short port;

    /*if (this->socket.receive(packet, ip, port) == sf::Socket::Done) {
        return 1;
    } else {
        return 0;
    }*/

	return this->socket.receive(packet, ip, port);
}

DataGhost NetworkGhostPlayer::GetPlayerData()
{
    DataGhost data = {
        VectorToQAngle(server->GetAbsOrigin(server->GetPlayer(GET_SLOT() + 1))),
        server->GetAbsAngles(server->GetPlayer(GET_SLOT() + 1))
    };

    return data;
}

GhostEntity* NetworkGhostPlayer::GetGhostByID(const sf::Uint32& ID)
{
    for (auto it : this->ghostPool) {
        if (it->ID == ID) {
            return it;
        }
    }
    return nullptr;
}

void NetworkGhostPlayer::SetPosAng(sf::Uint32& ID, Vector position, Vector angle)
{
    this->GetGhostByID(ID)->SetPosAng(position, angle);
}

//Update other players
void NetworkGhostPlayer::UpdateGhostsCurrentMap()
{
    for (auto& it : this->ghostPool) {
        if (engine->m_szLevelName == it->currentMap) {
            it->sameMap = true;
        } else {
            it->sameMap = false;
        }
    }
}

//Notify network of map change
void NetworkGhostPlayer::UpdateCurrentMap()
{
    sf::Packet packet;
    packet << HEADER::MAP_CHANGE << std::string(engine->m_szLevelName);
    this->tcpSocket.send(packet);
}

//Check if we can unpause when using sar_ghost_sync
bool NetworkGhostPlayer::AreGhostsOnSameMap()
{
    for (auto& it : this->ghostPool) {
        if (!it->sameMap) {
            return false;
        }
    }
    return true;
}

void NetworkGhostPlayer::StartThinking()
{
    if (this->runThread) { //Already running (level change, load save)
        this->pauseThread = false;
        this->waitForPaused.notify_one();
    } else { //First time we connect
        this->runThread = true;
        this->waitForPaused.notify_one();
        this->networkThread = std::thread(&NetworkGhostPlayer::NetworkThink, this);
        this->networkThread.detach();
        this->TCPThread = std::thread(&NetworkGhostPlayer::CheckConnection, this);
        this->TCPThread.detach();
    }
}

void NetworkGhostPlayer::PauseThinking()
{
    this->pauseThread = true;
}

//Called on another thread
void NetworkGhostPlayer::NetworkThink()
{
    std::map<unsigned int, sf::Packet> packet_queue;

    while (this->runThread || !this->pauseThread) {
        {
            std::unique_lock<std::mutex> lck(mutex); //Wait for the session to restart
            waitForPaused.wait(lck, [] { return (networkGhostPlayer->runThread) ? !networkGhostPlayer->pauseThread.load() : true; });
        }
        if (!this->runThread) { //If needs to stop the thread (game quit, disconnect)
            return;
        }

        //Send our position to server
        this->UpdatePlayer();

        //Update other players
        //int success = 1;
        sf::Socket::Status success = sf::Socket::Done;
        while (success == sf::Socket::Done) {
            /*if (success == sf::Socket::Done)
                console->Print("sf::Socket::Done");
            else if (success == sf::Socket::Error)
                console->Print("sf::Socket::Error");
            else if (success == sf::Socket::Partial)
                console->Print("sf::Socket::Partial");
            else if (success == sf::Socket::Disconnected)
                console->Print("sf::Socket::Disconnected");*/

            sf::Packet packet;
            sf::IpAddress ip;
            success = this->ReceivePacket(packet, ip, 50);
            /*if (success == sf::Socket::Done)
                console->Print(" sf::Socket::Done\n");
            else if (success == sf::Socket::Error)
                console->Print(" sf::Socket::Error\n");
            else if (success == sf::Socket::Partial)
                console->Print(" sf::Socket::Partial\n");
            else if (success == sf::Socket::Disconnected)
                console->Print(" sf::Socket::Disconnected\n");*/

            if (success == sf::Socket::Done) {
                packet_queue.insert({ ip.toInteger(), packet });
            }
        }

        for (auto& data_packet : packet_queue) {
            HEADER header;
            data_packet.second >> header;
            if (header == HEADER::UPDATE) { //Received new pos/ang or echo of our update
                sf::Uint32 ID;
                DataGhost data;
                data_packet.second >> ID >> data;
                auto ghost = this->GetGhostByID(ID);
                if (ghost != nullptr) {
                    if (ghost->sameMap && !pauseThread) { //" && !pauseThread" to verify the map is still loaded
                        if (ghost->ghost_entity == nullptr) {
                            ghost->Spawn(true, QAngleToVector(data.position), ghost->ghostType);
                        }
                        ghost->oldPos = ghost->newPos;
                        ghost->newPos = { { data.position.x, data.position.y, data.position.z + /*sar_ghost_height.GetFloat()*/ 16 }, { data.view_angle.x, data.view_angle.y, data.view_angle.z } };
                        ghost->loopTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - ghost->lastUpdate).count();
                        ghost->lastUpdate = std::chrono::steady_clock::now();
                    }
                }
            }
        }
        packet_queue.clear();
        std::this_thread::sleep_for(std::chrono::milliseconds(this->tickrate));
    }

    this->ghostPool.clear();
}

void NetworkGhostPlayer::CheckConnection()
{
    sf::SocketSelector tcpSelector;
    tcpSelector.add(this->tcpSocket);
    while (this->runThread) {
        sf::Packet packet;
        if (tcpSelector.wait(sf::seconds(2))) {
            sf::Socket::Status status = this->tcpSocket.receive(packet);
            if (status == sf::Socket::Done) {
                HEADER header;
                packet >> header;
                if (header == HEADER::CONNECT) {
                    sf::Uint32 ID;
                    std::string name;
                    DataGhost data;
                    std::string currentMap;
                    std::string ghostModelName;
                    packet >> ID >> name >> data >> currentMap >> ghostModelName;
                    this->ghostPool.push_back(this->SetupGhost(ID, name, data, currentMap, ghostModelName));
                    if (this->runThread) {
                        auto ghost = this->GetGhostByID(ID);
                        if (ghost->sameMap) {
                            ghost->Spawn(true, { 1, 1, 1 }, ghost->ghostType);
                        }
                    }
                    client->Chat(TextColor::LIGHT_GREEN, "Player %s has connected !\n", name.c_str());
                } else if (header == HEADER::DISCONNECT) {
                    sf::Uint32 ID;
                    packet >> ID;
                    int id = 0;
                    for (; id < this->ghostPool.size(); ++id) {
                        if (this->ghostPool[id]->ID == ID) {
                            client->Chat(TextColor::ORANGE, "Player %s has disconnected !\n", this->ghostPool[id]->name.c_str());
                            this->ghostPool[id]->Stop();
                            this->ghostPool.erase(this->ghostPool.begin() + id);
                            break;
                        }
                    }
                } else if (header == HEADER::STOP_SERVER) {
                    this->StopServer();
                } else if (header == HEADER::MAP_CHANGE) {
                    sf::Uint32 ID;
                    std::string newMap;
                    packet >> ID >> newMap;
                    auto ghost = this->GetGhostByID(ID);
                    client->Chat(TextColor::LIGHT_GREEN, "Player %s is now on %s\n", ghost->name.c_str(), newMap.c_str());
                    if (newMap == engine->m_szLevelName) {
                        ghost->sameMap = true;
                        if (sar_ghost_sync_maps.GetBool() && this->AreGhostsOnSameMap()) {
                            engine->ExecuteCommand("unpause");
                        }
                    } else {
                        ghost->sameMap = false;
                    }
                    ghost->currentMap = newMap;
                } else if (header == HEADER::MESSAGE) {
                    sf::Uint32 ID;
                    std::string message;
                    packet >> ID >> message;
                    std::string cmd;
                    if (ID == this->ip_client.toInteger()) {
                        client->Chat(TextColor::ORANGE, "%s : %s", this->name.c_str(), message.c_str());
                    } else {
                        client->Chat(TextColor::ORANGE, "%s : %s", this->GetGhostByID(ID)->name.c_str(), message.c_str());
                    }
                } else if (header == HEADER::PING) {
                    auto stop = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - this->start);
                    console->Print("Pong ! %lld ms\n", elapsed.count());
                } else if (header == HEADER::COUNTDOWN) {
                    sf::Uint8 step;
                    packet >> step;
                    if (step == 0) {
                        std::string preCommand;
                        std::string postCommand;
                        sf::Uint32 time;
                        packet >> time >> preCommand >> postCommand;
                        engine->ExecuteCommand(preCommand.c_str());
                        this->commandPostCountdown = postCommand;

                        sf::Packet packet_confirm;
                        packet_confirm << HEADER::COUNTDOWN << sf::Uint8(1);
                        tcpSocket.send(packet_confirm);
                        this->SetupCountdown(time);
                    } else if (step == 1) {
                        this->Countdown();
                    }
                } else if (header == HEADER::MODEL_CHANGE) {
                    sf::Uint32 ID;
                    std::string modelName;
                    packet >> ID >> modelName;
                    auto ghost = this->GetGhostByID(ID);
                    if (this->defaultGhostType == 2) {
                        ghost->ChangeModel(modelName);
                    }
                }
            } else if (status == sf::Socket::Disconnected) {
                client->Chat(TextColor::ORANGE, "Connexion has been interrupted ! You have been disconnected !\n");
                this->Disconnect();
            }
        }
    }
}

GhostEntity* NetworkGhostPlayer::SetupGhost(sf::Uint32& ID, std::string name, DataGhost& data, std::string& currentMap, std::string& modelName)
{
    GhostEntity* tmp_ghost = new GhostEntity(this->defaultGhostType);
    tmp_ghost->name = name;
    tmp_ghost->ID = ID;
    tmp_ghost->currentMap = currentMap;
    tmp_ghost->sameMap = (currentMap == engine->m_szLevelName);
    tmp_ghost->ChangeModel(modelName);
    return tmp_ghost;
}

void NetworkGhostPlayer::UpdatePlayer()
{
    HEADER header = HEADER::UPDATE;
    DataGhost dataGhost = this->GetPlayerData();
    sf::Packet packet;
    packet << header << dataGhost;
    this->socket.send(packet, this->ip_server, this->port_server);
}

void NetworkGhostPlayer::ClearGhosts()
{
    for (auto& ghost : this->ghostPool) {
        ghost->Stop();
    }
}

void NetworkGhostPlayer::SetupCountdown(sf::Uint32 time)
{
    this->countdown = time;
    this->start = std::chrono::steady_clock::now();
}

void NetworkGhostPlayer::Countdown()
{
    auto ping = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->start);
    this->startCountdown = std::chrono::steady_clock::now() + std::chrono::seconds(3) - ping; // Start the coutdown 3s after the command
    this->isCountdownReady = true;
}

//Commands

CON_COMMAND(sar_ghost_connect_to_server, "Connect to the server : <ip address> <port> :\n"
                                         "ex: 'localhost 53000' - '127.0.0.1 53000' - 89.10.20.20 53000'.")
{
    if (args.ArgC() <= 2) {
        console->Print(sar_ghost_connect_to_server.ThisPtr()->m_pszHelpString);
        return;
    }

    if (networkGhostPlayer->IsConnected()) {
        console->Warning("Already connected to the server !\n");
        return;
    }

    std::string ip = args[1];
    if (ip.find("10.") == 0 || ip.find("192.168.") == 0) { //If local
        networkGhostPlayer->ip_client = sf::IpAddress::getLocalAddress();
    } else {
        int local = false;
        for (int i = 16; i < 32; ++i) {
            if (ip.find("172." + std::to_string(i)) == 0) {
                networkGhostPlayer->ip_client = sf::IpAddress::getLocalAddress();
                local = true;
                break;
            }
        }
        if (local == false) {
            networkGhostPlayer->ip_client = sf::IpAddress::getPublicAddress();
        }
    }

    networkGhostPlayer->ConnectToServer(args[1], std::atoi(args[2]));
}

CON_COMMAND(sar_ghost_ping, "Send ping\n")
{
    sf::Packet packet;
    packet << HEADER::PING;

    networkGhostPlayer->start = std::chrono::steady_clock::now();
    networkGhostPlayer->tcpSocket.send(packet);
}

CON_COMMAND(sar_ghost_disconnect, "Disconnect the player from the server\n")
{

    if (!networkGhostPlayer->IsConnected()) {
        console->Warning("You are not connected to a server !\n");
        return;
    }

    networkGhostPlayer->Disconnect();
    client->Chat(TextColor::ORANGE, "You have successfully been disconnected !\n");
}

CON_COMMAND(sar_ghost_name, "Name that will be displayed\n")
{
    if (args.ArgC() <= 1) {
        console->Print(sar_ghost_name.ThisPtr()->m_pszHelpString);
        return;
    }
    networkGhostPlayer->name = args[1];
}

CON_COMMAND(sar_ghost_tickrate, "Adjust the tickrate\n")
{
    if (args.ArgC() <= 1) {
        console->Print(sar_ghost_tickrate.ThisPtr()->m_pszHelpString);
        return;
    }
    networkGhostPlayer->tickrate = std::chrono::milliseconds(std::atoi(args[1]));
}

CON_COMMAND(sar_ghost_countdown, "Start a countdown\n")
{
    if (args.ArgC() < 2) {
        console->Print(sar_ghost_tickrate.ThisPtr()->m_pszHelpString);
        return;
    } else if (args.ArgC() >= 2) {
        sf::Packet packet;
        packet << HEADER::COUNTDOWN << sf::Uint8(0) << sf::Uint32(std::atoi(args[1])) << std::string("") << std::string(""); //Players can't use pre/post commands
        networkGhostPlayer->tcpSocket.send(packet);
    }
}

CON_COMMAND(sar_ghost_message, "Send a message to other players\n")
{
    if (args.ArgC() <= 1) {
        console->Print(sar_ghost_message.ThisPtr()->m_pszHelpString);
        return;
    }
    sf::Packet packet;
    packet << HEADER::MESSAGE;
    std::string message = "";
    for (int i = 1; i < args.ArgC(); ++i) {
        message += args[i];
    }
    packet << message;
    networkGhostPlayer->tcpSocket.send(packet);
}