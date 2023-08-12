// This *has* to come first because <winsock2> doesn't like being
// imported after <windows>. I fucking hate this platform
#ifdef _WIN32
#	include <winsock2.h>
#	include <ws2tcpip.h>
#else
#	include <sys/socket.h>
#	include <sys/select.h>
#	include <arpa/inet.h>
#	include <netinet/in.h>
#	include <unistd.h>
#endif

#ifndef _WIN32
#	define SOCKET int
#	define INVALID_SOCKET -1
#	define SOCKET_ERROR -1
#	define closesocket close
#	define WSACleanup() (void)0
#endif

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
#define DEFAULT_TAS_SERVER_SOCKET 6555

Variable sar_tas_protocol_reconnect_delay("sar_tas_protocol_reconnect_delay", "0", 0, 
	"A number of seconds after which reconnection to TAS protocol server should be made.\n"
	"0 means no reconnect attempts will be made.\n");

using namespace TasProtocol;

static SOCKET g_listen_sock = INVALID_SOCKET;
static std::vector<ConnectionData> g_connections;
static std::atomic<bool> g_should_stop;
static bool g_should_run = false;
static bool g_stopped_manually = true;
static std::atomic<bool> g_is_server;
static std::chrono::high_resolution_clock::time_point g_last_connection_attemt_timestamp;

static std::string g_client_ip;
static int g_client_port;
static int g_server_port;
static std::mutex g_conn_data_mutex;

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
		uint8_t packetId = cl.cmdbuf[0];

		switch (packetId) {
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
		case RECV_SET_CONT_ENTITY_INFO:
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

				if (packetId == RECV_SET_CONT_ENTITY_INFO) {
					cl.contInfoEntSelector = entSelector;
				} else {
					SendEntityInfo(cl, entSelector);
				}
			}
			break;

		default:
			return false; // Bad command - disconnect
		}
	}
}

static bool attemptToInitializeServer() {
	if (!g_is_server.load()) return false;

	g_conn_data_mutex.lock();
	auto server_port = g_server_port;
	g_conn_data_mutex.unlock();

	g_listen_sock = socket(AF_INET6, SOCK_STREAM, 0);
	if (g_listen_sock == INVALID_SOCKET) {
		THREAD_PRINT("Could not initialize TAS server: socket creation failed\n");
		return false;
	}

	// why tf is this enabled by default on Windows
	int v6only = 0;
	setsockopt(g_listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&v6only, sizeof v6only);

	struct sockaddr_in6 saddr {
		AF_INET6,
			htons(server_port),
			0,
			in6addr_any,
			0,
	};

	if (bind(g_listen_sock, (struct sockaddr *)&saddr, sizeof saddr) == SOCKET_ERROR) {
		THREAD_PRINT("Could not initialize TAS server: socket bind failed\n");
		closesocket(g_listen_sock);
		return false;
	}

	if (listen(g_listen_sock, 4) == SOCKET_ERROR) {
		THREAD_PRINT("Could not initialize TAS server: socket listen failed\n");
		closesocket(g_listen_sock);
		return false;
	}

	THREAD_PRINT("TAS server initialized on port %d.\n", server_port);

	return true;
}

static bool attemptConnectionToServer() {
	if (g_is_server.load()) return false;

	g_conn_data_mutex.lock();
	std::string ip = g_client_ip;
	int port = g_client_port;
	g_conn_data_mutex.unlock();

	auto clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == SOCKET_ERROR) {
		THREAD_PRINT("Could not connect to TAS protocol server: socket creation failed\n");
		closesocket(clientSocket);
		return false;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);

	if (inet_pton(AF_INET, g_client_ip.c_str(), &serverAddr.sin_addr) <= 0) {
		THREAD_PRINT("Could not connect to TAS protocol server: invalid address\n");
		closesocket(clientSocket);
		return false;
	}

	if (connect(clientSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1) {
		THREAD_PRINT("Could not connect to TAS protocol server: connection failed.\n");
		closesocket(clientSocket);
		return false;
	}

	g_connections.push_back({clientSocket, {}});
	fullUpdate(g_connections[g_connections.size() - 1], true);
	THREAD_PRINT("Successfully connected to TAS server %s:%d.\n", ip.c_str(), port);

	return true;
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

static void processConnections(bool is_server) {
	fd_set set;
	FD_ZERO(&set);

	SOCKET max = g_listen_sock;

	if (is_server) {
		FD_SET(g_listen_sock, &set);
	}
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

	if (is_server && FD_ISSET(g_listen_sock, &set)) {
		SOCKET cl = accept(g_listen_sock, nullptr, nullptr);
		if (cl != INVALID_SOCKET) {
			g_connections.push_back({ cl, {} });
			THREAD_PRINT("A controller connected to TAS server. Number of controllers: %d\n", g_connections.size());
			fullUpdate(g_connections[g_connections.size() - 1], true);
		}
	}

	for (size_t i = 0; i < g_connections.size(); ++i) {
		auto &cl = g_connections[i];

		if (!FD_ISSET(cl.sock, &set)) continue;

		if (!receiveFromConnection(cl)) {
			g_connections.erase(g_connections.begin() + i);
			--i;

			if (is_server) {
				THREAD_PRINT("A controller disconnected from TAS server. Number of controllers: %d\n", g_connections.size());
			}
		}
	}
}

static void mainThread() {
	THREAD_PRINT("Starting TAS protocol connection\n");

#ifdef _WIN32
	WSADATA wsa_data;
	int err = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (err){
		THREAD_PRINT("Could not initialize TAS protocol: WSAStartup failed (%d)\n", err);
		g_should_stop.store(true);
		return;
	}
#endif

	bool is_server = g_is_server.load();

	if (is_server && !attemptToInitializeServer()) {
		WSACleanup();
		g_should_stop.store(true);
		return;
	}
	if (!is_server && !attemptConnectionToServer()) {
		WSACleanup();
		g_should_stop.store(true);
		return;
	}

	while (!g_should_stop.load()) {
		processConnections(is_server);
		update();

		if (g_connections.size() == 0 && !is_server) {
			break;
		}
	}

	if (is_server) {
		THREAD_PRINT("Stopping TAS server\n");
	} else {
		THREAD_PRINT("Stopping TAS client\n");
	}

	for (auto &cl : g_connections) {
		closesocket(cl.sock);
	}

	closesocket(g_listen_sock);
	WSACleanup();
	g_should_stop.store(true);
}

static std::thread g_net_thread;
static bool g_running;

static void restart() {
	g_should_stop.store(true);
	g_should_run = true;
	g_stopped_manually = true;
}

ON_EVENT(FRAME) {
	auto now = std::chrono::high_resolution_clock::now();

	if (!g_running && !g_stopped_manually && sar_tas_protocol_reconnect_delay.GetBool() && !g_is_server.load()) {
		auto duration = ((std::chrono::duration<float>)(now - g_last_connection_attemt_timestamp)).count();
		if (duration > sar_tas_protocol_reconnect_delay.GetFloat()) {
			g_should_run = true;
			console->Print("Attempting to reconnect to the TAS protocol server\n");
		}
	}

	if (g_running && g_should_stop.load()) {
		if (g_net_thread.joinable()) g_net_thread.join();
		g_running = false;
	} else if (!g_running && g_should_run) {
		g_should_stop.store(false);
		g_net_thread = std::thread(mainThread);
		g_running = true;
		g_should_run = false;
		g_stopped_manually = false;

		g_last_connection_attemt_timestamp = now;
	}
}

ON_EVENT_P(SAR_UNLOAD, -100) {
	g_should_stop.store(true);
	if (g_net_thread.joinable()) g_net_thread.join();
}

void TasProtocol::SetStatus(Status s) {
	g_status_mutex.lock();
	g_current_status = s;
	g_status_mutex.unlock();

	if (s.active) {
		for (auto &cl : g_connections) {
			if (cl.contInfoEntSelector.length() == 0) continue;

			std::vector<uint8_t> buf{SEND_CURRENT_TICK};
			encodeRaw32(buf, s.playback_tick);
			send(cl.sock, (const char *)buf.data(), buf.size(), 0);
			SendEntityInfo(cl, cl.contInfoEntSelector);
		}
	}
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

void TasProtocol::SendEntityInfo(TasProtocol::ConnectionData &conn, std::string entSelector) {
	std::vector<uint8_t> buf;
	buf.push_back(SEND_ENTITY_INFO);

	CEntInfo *entInfo = entityList->QuerySelector(entSelector.c_str());
	if (entInfo != NULL) {
		buf.push_back(1);
		ServerEnt *ent = (ServerEnt *)entInfo->m_pEntity;

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

	send(conn.sock, (const char *)buf.data(), buf.size(), 0);
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
            "ex: '127.0.0.1 5666' - '89.10.20.20 5666'.\n") {
	if (args.ArgC() < 2 || args.ArgC() > 3) {
		return console->Print(sar_tas_protocol_connect.ThisPtr()->m_pszHelpString);
	}

	g_conn_data_mutex.lock();

	g_client_ip = args[1];
	g_client_port = args.ArgC() >= 3 ? std::atoi(args[2]) : DEFAULT_TAS_CLIENT_SOCKET;

	g_conn_data_mutex.unlock();

	g_is_server.store(false);

	restart();
}

CON_COMMAND(sar_tas_protocol_server,
            "sar_tas_protocol_server [port] - starts a TAS protocol server. Port is 6555 by default.\n") {
	if (args.ArgC() < 1 || args.ArgC() > 2) {
		return console->Print(sar_tas_protocol_server.ThisPtr()->m_pszHelpString);
	}
	g_conn_data_mutex.lock();
	g_server_port = args.ArgC() >= 2 ? std::atoi(args[1]) : DEFAULT_TAS_SERVER_SOCKET;
	g_conn_data_mutex.unlock();

	g_is_server.store(true);

	restart();
}

CON_COMMAND(sar_tas_protocol_stop,
            "sar_tas_protocol_stop - stops every TAS protocol related connection.\n") {
	g_should_stop.store(true);
	g_stopped_manually = true;
}
