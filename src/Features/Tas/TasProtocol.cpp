#include "TasProtocol.hpp"
#include "TasPlayer.hpp"
#include "Event.hpp"
#include "Scheduler.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Features/PlayerTrace.hpp"
#include "Features/EntityList.hpp"

#include <vector>
#include <deque>
#include <thread>
#include <atomic>
#include <mutex>
#include <filesystem>

#define DEFAULT_TAS_CLIENT_SOCKET 6555

using namespace TasProtocol;

Variable sar_tas_protocol("sar_tas_protocol", "0", 0, "Enable the remote TAS controller connection protocol. Value higher than 1 replaces port to listen for connections on (6555 by default).\n");



static SOCKET g_listen_sock = INVALID_SOCKET;
static std::vector<ConnectionData> g_connections;
static std::atomic<bool> g_should_stop;

static std::string g_client_ip;
static int g_client_port;
static bool g_attempt_client_connection;
static std::mutex g_client_data_mutex;

static Status g_last_status;
static Status g_current_status;
static int g_last_debug_tick;
static int g_current_debug_tick;
static std::mutex g_status_mutex;

static uint32_t popRaw32(std::deque<uint8_t> &buf) {
	uint32_t val = 0;
	for (int i = 0; i < 4; ++i) {
		val <<= 8;
		val |= buf[0];
		buf.pop_front();
	}
	return val;
}

static void encodeRaw32(std::vector<uint8_t> &buf, uint32_t val) {
	buf.push_back((val >> 24) & 0xFF);
	buf.push_back((val >> 16) & 0xFF);
	buf.push_back((val >> 8)  & 0xFF);
	buf.push_back((val >> 0)  & 0xFF);
}

// I know this is ugly, but I think copy pasting the above code is
// stupider so whatever
static void encodeRawFloat(std::vector<uint8_t> &buf, float val) {
	encodeRaw32(buf, *(uint32_t *)&val);
}

static void sendAll(const std::vector<uint8_t> &buf) {
	for (auto &cl : g_connections) {
		send(cl.sock, (const char *)buf.data(), buf.size(), 0);
	}
}

static void fullUpdate(TasProtocol::ConnectionData &cl, bool first_packet = false) {
	std::vector<uint8_t> buf;

	if (first_packet) {
		buf.push_back(SEND_GAME_LOCATION);

		std::string dir = std::filesystem::current_path().string();
		std::replace(dir.begin(), dir.end(), '\\', '/');
		encodeRaw32(buf, (uint32_t)dir.size());
		for (char c : dir) {
			buf.push_back(c);
		}
	}

	buf.push_back(SEND_PLAYBACK_RATE);

	union { float f; int i; } rate = { g_last_status.playback_rate };
	encodeRaw32(buf, rate.i);

	if (g_last_status.active) {
		buf.push_back(SEND_ACTIVE);
		encodeRaw32(buf, g_last_status.tas_path[0].size());
		for (char c : g_last_status.tas_path[0]) {
			buf.push_back(c);
		}
		encodeRaw32(buf, g_last_status.tas_path[1].size());
		for (char c : g_last_status.tas_path[1]) {
			buf.push_back(c);
		}

		// state
		switch (g_last_status.playback_state) {
		case PlaybackState::PLAYING:
			buf.push_back(SEND_PLAYING);
			break;
		case PlaybackState::PAUSED:
			buf.push_back(SEND_PAUSED);
			break;
		case PlaybackState::SKIPPING:
			buf.push_back(SEND_SKIPPING);
			break;
		}

		buf.push_back(SEND_CURRENT_TICK);
		encodeRaw32(buf, g_last_status.playback_tick);
	} else {
		buf.push_back(SEND_INACTIVE);
	}

	buf.push_back(SEND_DEBUG_TICK);
	encodeRaw32(buf, (uint32_t)g_last_debug_tick);

	send(cl.sock, (const char *)buf.data(), buf.size(), 0);
}

static void update() {
	g_status_mutex.lock();
	Status status = g_current_status;
	int debug_tick = g_current_debug_tick;
	g_status_mutex.unlock();

	if (status.active != g_last_status.active || status.tas_path[0] != g_last_status.tas_path[0] || status.tas_path[1] != g_last_status.tas_path[1]) {
		// big change; we might as well just do a full update
		g_last_status = status;
		g_last_debug_tick = debug_tick;
		for (auto &cl : g_connections) fullUpdate(cl);
		return;
	}

	if (status.playback_rate != g_last_status.playback_rate) {
		union { float f; int i; } rate = { status.playback_rate };

		std::vector<uint8_t> buf{SEND_PLAYBACK_RATE};
		encodeRaw32(buf, rate.i);
		sendAll(buf);

		g_last_status.playback_rate = status.playback_rate;
	}

	if (status.active && status.playback_state != g_last_status.playback_state) {
		switch (status.playback_state) {
		case PlaybackState::PLAYING:
			sendAll({ SEND_PLAYING });
			break;
		case PlaybackState::PAUSED:
			sendAll({ SEND_PAUSED });
			break;
		case PlaybackState::SKIPPING:
			sendAll({ SEND_SKIPPING });
			break;
		}

		g_last_status.playback_state = status.playback_state;
	}

	if (status.active && status.playback_tick != g_last_status.playback_tick) {
		std::vector<uint8_t> buf{ SEND_CURRENT_TICK};
		encodeRaw32(buf, status.playback_tick);
		sendAll(buf);

		g_last_status.playback_tick = status.playback_tick;
	}

	if (debug_tick != g_last_debug_tick) {
		std::vector<uint8_t> buf{ SEND_DEBUG_TICK};
		encodeRaw32(buf, (uint32_t)debug_tick);
		sendAll(buf);

		g_last_debug_tick = debug_tick;
	}
}

static bool processCommands(ConnectionData &cl) {
	while (true) {
		if (cl.cmdbuf.size() == 0) return true;

		size_t extra = cl.cmdbuf.size() - 1;

		switch (cl.cmdbuf[0]) {
		case RECV_PLAY_SCRIPT:
			if (extra < 8) return true;
			{
				std::deque<uint8_t> copy = cl.cmdbuf;

				copy.pop_front();

				uint32_t len1 = popRaw32(copy);
				if (extra < 8 + len1) return true;

				std::string filename1;
				for (size_t i = 0; i < len1; ++i) {
					filename1 += copy[0];
					copy.pop_front();
				}

				uint32_t len2 = popRaw32(copy);
				if (extra < 8 + len1 + len2) return true;

				std::string filename2;
				for (size_t i = 0; i < len2; ++i) {
					filename2 += copy[0];
					copy.pop_front();
				}

				// We actually had everything we needed, so switch to the modified buffer
				cl.cmdbuf = copy; 

				Scheduler::OnMainThread([=](){
					tasPlayer->PlayFile(filename1, filename2);
				});
			}
			break;

		case RECV_STOP:
			cl.cmdbuf.pop_front();
			Scheduler::OnMainThread([=](){
				tasPlayer->Stop(true);
			});
			break;

		case RECV_PLAYBACK_RATE:
			if (extra < 4) return true;
			cl.cmdbuf.pop_front();
			{
				union { uint32_t i; float f; } rate = { popRaw32(cl.cmdbuf) };
				Scheduler::OnMainThread([=](){
					sar_tas_playback_rate.SetValue(rate.f);
				});
			}
			break;

		case RECV_RESUME:
			cl.cmdbuf.pop_front();
			Scheduler::OnMainThread([=](){
				tasPlayer->Resume();
			});
			break;

		case RECV_PAUSE: 
			cl.cmdbuf.pop_front();
			Scheduler::OnMainThread([=](){
				tasPlayer->Pause();
			});
			break;

		case RECV_FAST_FORWARD:
			if (extra < 5) return true;
			cl.cmdbuf.pop_front();
			{
				int tick = popRaw32(cl.cmdbuf);
				bool pause_after = cl.cmdbuf[0];
				cl.cmdbuf.pop_front();
				Scheduler::OnMainThread([=](){
					sar_tas_skipto.SetValue(tick);
					if (pause_after) sar_tas_pauseat.SetValue(tick);
				});
			}
			break;

		case RECV_SET_PAUSE_TICK:
			if (extra < 4) return true;
			cl.cmdbuf.pop_front();
			{
				int tick = popRaw32(cl.cmdbuf);
				Scheduler::OnMainThread([=](){
					sar_tas_pauseat.SetValue(tick);
				});
			}
			break;

		case RECV_ADVANCE_TICK:
			cl.cmdbuf.pop_front();
			Scheduler::OnMainThread([](){
				tasPlayer->AdvanceFrame();
			});
			break;

		case RECV_PLAY_SCRIPT_PROTOCOL:
			if (extra < 16) return true;
			{
				std::deque<uint8_t> copy = cl.cmdbuf;

				copy.pop_front();

				std::string data[4];
				uint32_t size_total = 0;
				for (int i = 0; i < 4; ++i) {
					uint32_t len = popRaw32(copy);
					size_total += len;
					if (extra < 16 + size_total) return true;

					for (size_t j = 0; j < len; ++j) {
						data[i] += copy[0];
						copy.pop_front();
					}
				}

				cl.cmdbuf = copy;  // We actually had everything we needed, so switch to the modified buffer

				Scheduler::OnMainThread([=]() {
					tasPlayer->PlayScript(data[0], data[1], data[2], data[3]);
				});
			}
			break;
		case RECV_ENTITY_INFO:
			if (extra < 4) return true;
			{
				std::deque<uint8_t> copy = cl.cmdbuf;
				copy.pop_front();

				uint32_t len = popRaw32(copy);
				if (extra < 4 + len) return true;

				std::string entSelector;
				for (size_t i = 0; i < len; ++i) {
					entSelector += copy[0];
					copy.pop_front();
				}

				cl.cmdbuf = copy;

				std::vector<uint8_t> buf;
				buf.push_back(SEND_ENTITY_INFO);

				CEntInfo *entInfo = entityList->QuerySelector(entSelector.c_str());
				if (entInfo != NULL) {
					buf.push_back(1);
					ServerEnt* ent = (ServerEnt*)entInfo->m_pEntity;

					Vector position = ent->abs_origin();
					encodeRawFloat(buf, position.x);
					encodeRawFloat(buf, position.y);
					encodeRawFloat(buf, position.z);

					QAngle angles = ent->abs_angles();
					encodeRawFloat(buf, angles.x);
					encodeRawFloat(buf, angles.y);
					encodeRawFloat(buf, angles.z);

					Vector velocity = ent->abs_velocity();
					encodeRawFloat(buf, velocity.x);
					encodeRawFloat(buf, velocity.y);
					encodeRawFloat(buf, velocity.z);
					
				} else {
					buf.push_back(0);
				}

				send(cl.sock, (const char *)buf.data(), buf.size(), 0);
			}
			break;

		default:
			return false; // Bad command - disconnect
		}
	}
}

static void attemptConnectionToServer() {
	g_client_data_mutex.lock();

	bool shouldConnect = g_attempt_client_connection;
	g_attempt_client_connection = false;
	std::string ip = g_client_ip;
	int port = g_client_port;

	g_client_data_mutex.unlock();

	if (!shouldConnect) return;

	auto clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == -1) {
		THREAD_PRINT("Could not connect to TAS protocol server: socket creation failed\n");
		closesocket(clientSocket);
		return;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);

	if (inet_pton(AF_INET, g_client_ip.c_str(), &serverAddr.sin_addr) <= 0) {
		THREAD_PRINT("Could not connect to TAS protocol server: invalid address\n");
		closesocket(clientSocket);
		return;
	}

	if (connect(clientSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1) {
		THREAD_PRINT("Could not connect to TAS protocol server: connection failed.\n");
		closesocket(clientSocket);
		return;
	}

	g_connections.push_back({clientSocket, {}});
	fullUpdate(g_connections[g_connections.size() - 1], true);
	THREAD_PRINT("Successfully connected to TAS server %s:%d.\n", ip.c_str());
}

static bool receiveFromConnection(TasProtocol::ConnectionData &cl) {
	char buf[1024];
	int len = recv(cl.sock, buf, sizeof buf, 0);

	if (len == 0 || len == SOCKET_ERROR) {  // Connection closed or errored
		return false;
	}

	cl.cmdbuf.insert(cl.cmdbuf.end(), std::begin(buf), std::begin(buf) + len);

	if (!processCommands(cl)) {
		// Client sent a bad command; terminate connection
		closesocket(cl.sock);
		return false;
	}

	return true;
}

static void processConnections() {
	fd_set set;
	FD_ZERO(&set);

	SOCKET max = g_listen_sock;

	FD_SET(g_listen_sock, &set);
	for (auto client : g_connections) {
		FD_SET(client.sock, &set);
		if (max < client.sock) max = client.sock;
	}

	// 0.05s timeout
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 50000;

	int nsock = select(max + 1, &set, nullptr, nullptr, &tv);
	if (nsock == SOCKET_ERROR || !nsock) {
		return;
	}

	if (FD_ISSET(g_listen_sock, &set)) {
		SOCKET cl = accept(g_listen_sock, nullptr, nullptr);
		if (cl != INVALID_SOCKET) {
			g_connections.push_back({ cl, {} });
			fullUpdate(g_connections[g_connections.size() - 1], true);
		}
	}

	for (size_t i = 0; i < g_connections.size(); ++i) {
		auto &cl = g_connections[i];

		if (!FD_ISSET(cl.sock, &set)) continue;

		if (!receiveFromConnection(cl)) {
			g_connections.erase(g_connections.begin() + i);
			--i;
		}
	}
}

static void mainThread(int tas_server_port) {
	THREAD_PRINT("Starting TAS server\n");

#ifdef _WIN32
	WSADATA wsa_data;
	int err = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (err){
		THREAD_PRINT("Could not initialize TAS server: WSAStartup failed (%d)\n", err);
		return;
	}
#endif

	g_listen_sock = socket(AF_INET6, SOCK_STREAM, 0);
	if (g_listen_sock == INVALID_SOCKET) {
		THREAD_PRINT("Could not initialize TAS server: socket creation failed\n");
		WSACleanup();
		return;
	}

	// why tf is this enabled by default on Windows
	int v6only = 0;
	setsockopt(g_listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&v6only, sizeof v6only);

	struct sockaddr_in6 saddr{
		AF_INET6,
		htons(tas_server_port),
		0,
		in6addr_any,
		0,
	};

	if (bind(g_listen_sock, (struct sockaddr *)&saddr, sizeof saddr) == SOCKET_ERROR) {
		THREAD_PRINT("Could not initialize TAS server: socket bind failed\n");
		closesocket(g_listen_sock);
		WSACleanup();
		return;
	}

	if (listen(g_listen_sock, 4) == SOCKET_ERROR) {
		THREAD_PRINT("Could not initialize TAS server: socket listen failed\n");
		closesocket(g_listen_sock);
		WSACleanup();
		return;
	}

	while (!g_should_stop.load()) {
		attemptConnectionToServer();
		processConnections();
		update();
	}

	THREAD_PRINT("Stopping TAS server\n");

	for (auto &cl : g_connections) {
		closesocket(cl.sock);
	}

	closesocket(g_listen_sock);
	WSACleanup();
}

static std::thread g_net_thread;
static bool g_running;

ON_EVENT(FRAME) {
	int tas_server_port = sar_tas_protocol.GetInt() == 1 ? DEFAULT_TAS_CLIENT_SOCKET : sar_tas_protocol.GetInt();
	bool should_run = sar_tas_protocol.GetBool();

	if (g_running && !should_run) {
		g_should_stop.store(true);
		if (g_net_thread.joinable()) g_net_thread.join();
		g_running = false;
	} else if (!g_running && should_run) {
		g_should_stop.store(false);
		g_net_thread = std::thread(mainThread, tas_server_port);
		g_running = true;
	}
}

ON_EVENT_P(SAR_UNLOAD, -100) {
	sar_tas_protocol.SetValue(false);
	g_should_stop.store(true);
	if (g_net_thread.joinable()) g_net_thread.join();
}

void TasProtocol::SetStatus(Status s) {
	g_status_mutex.lock();
	g_current_status = s;
	g_status_mutex.unlock();
}

void TasProtocol::SendProcessedScript(uint8_t slot, std::string scriptString) {
	std::vector<uint8_t> buf;

	buf.push_back(SEND_PROCESSED_SCRIPT);

	// slot
	buf.push_back(slot);

	// script
	encodeRaw32(buf, (uint32_t)scriptString.size());
	for (char c : scriptString) {
		buf.push_back(c);
	}

	sendAll(buf);
}

ON_EVENT(FRAME) {
	g_status_mutex.lock();
	if (g_current_status.active) {
		g_current_debug_tick = g_current_status.playback_tick;
	} else {
		g_current_debug_tick = playerTrace->GetTasTraceTick();
	}
	g_status_mutex.unlock();
}

CON_COMMAND(sar_tas_protocol_connect,
            "sar_tas_protocol_connect <ip address> <port> - connect to the TAS protocol server.\n"
            "ex: 'localhost 6555' - '127.0.0.1 6555' - '89.10.20.20 6555'.\n") {
	if (args.ArgC() < 2 || args.ArgC() > 3) {
		return console->Print(sar_tas_protocol_connect.ThisPtr()->m_pszHelpString);
	}

	sar_tas_protocol.SetValue(1);

	g_client_data_mutex.lock();

	g_client_ip = args[1];
	g_client_port = args.ArgC() >= 3 ? std::atoi(args[2]) : DEFAULT_TAS_CLIENT_SOCKET;
	g_attempt_client_connection = true;

	g_client_data_mutex.unlock();
}
