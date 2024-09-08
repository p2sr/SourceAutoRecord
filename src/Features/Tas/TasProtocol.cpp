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
static std::chrono::high_resolution_clock::time_point g_last_connection_attempt_timestamp;

static std::string g_client_ip;
static int g_client_port;
static int g_server_port;
static std::mutex g_conn_data_mutex;

static Status g_last_status;
static Status g_current_status;
static int g_last_debug_tick;
static int g_current_debug_tick;
static std::mutex g_status_mutex;

static bool popByte(std::deque<uint8_t> &buf, uint8_t &val) {
	if (buf.size() < 1) return false;
	val = buf[0];
	buf.pop_front();
	return true;
}

// static void encodeByte(std::deque<uint8_t>& buf, uint8_t val) {
// 	buf.push_back(val);
// }

static bool popRaw32(std::deque<uint8_t> &buf, uint32_t& val) {

	if (buf.size() < 4) return false;

	val = 0;
	for (int i = 0; i < 4; ++i) {
		val <<= 8;
		val |= buf[0];
		buf.pop_front();
	}

	return true;
}

static void encodeRaw32(std::vector<uint8_t> &buf, uint32_t val) {
	buf.push_back((val >> 24) & 0xFF);
	buf.push_back((val >> 16) & 0xFF);
	buf.push_back((val >> 8)  & 0xFF);
	buf.push_back((val >> 0)  & 0xFF);
}

static bool popRawFloat(std::deque<uint8_t>& buf, float& val) {
	return popRaw32(buf, (uint32_t&)val);
}

static void encodeRawFloat(std::vector<uint8_t> &buf, float val) {
	encodeRaw32(buf, *(uint32_t *)&val);
}

static bool popString(std::deque<uint8_t> &buf, std::string &val) {
	uint32_t len;
	if(!popRaw32(buf, len)) return false;
	if (buf.size() < len) return false;

	for (size_t i = 0; i < len; ++i) {
		val += buf[0];
		buf.pop_front();
	}
	return true;
}

static void encodeString(std::vector<uint8_t>& buf, std::string val) {
	encodeRaw32(buf, (uint32_t)val.size());
	for (char c : val) {
		buf.push_back(c);
	}
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
		encodeString(buf, dir);
	}

	buf.push_back(SEND_PLAYBACK_RATE);

	union { float f; int i; } rate = { g_last_status.playback_rate };
	encodeRaw32(buf, rate.i);

	if (g_last_status.active) {
		buf.push_back(SEND_ACTIVE);
		encodeString(buf, g_last_status.tas_path[0]);
		encodeString(buf, g_last_status.tas_path[1]);

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

// reads a single tas protocol command
// returns 0 if command has been handled properly
// returns 1 if there's no data to fully process a command
// returns 2 if bad command is received
static int processCommand(ConnectionData &cl) {
	std::deque<uint8_t> copy = cl.cmdbuf;

	uint8_t packetId;
	if (popByte(cl.cmdbuf, packetId)) switch (packetId) {

	case RECV_PLAY_SCRIPT: {
		std::string filename1;
		std::string filename2;

		if (!popString(cl.cmdbuf, filename1)) break;
		if (!popString(cl.cmdbuf, filename2)) break;

		Scheduler::OnMainThread([=](){
			tasPlayer->PlayFile(filename1, filename2);
		});
		
		return 0;
	}
	case RECV_STOP: {
		Scheduler::OnMainThread([=](){
			tasPlayer->Stop(true);
		});

		return 0;
	}
	case RECV_PLAYBACK_RATE: {
		float rate;

		popRawFloat(cl.cmdbuf, rate);

		Scheduler::OnMainThread([=](){
			sar_tas_playback_rate.SetValue(rate);
		});
		
		return 0;
	}
	case RECV_RESUME: {
		Scheduler::OnMainThread([=]() {
			tasPlayer->Resume();
		});

		return 0;
	}
	case RECV_PAUSE: {
		Scheduler::OnMainThread([=]() {
			tasPlayer->Pause();
		});

		return 0;
	}
	case RECV_FAST_FORWARD: {
		int tick;
		bool pause_after;

		if (!popRaw32(cl.cmdbuf, (uint32_t &)tick)) break;
		if (!popByte(cl.cmdbuf, (uint8_t &)pause_after)) break;
		
		Scheduler::OnMainThread([=]() {
			sar_tas_skipto.SetValue(tick);
			if (pause_after) sar_tas_pauseat.SetValue(tick);
		});
		return 0;
	}
	case RECV_SET_PAUSE_TICK: {
		int tick;

		if (!popRaw32(cl.cmdbuf, (uint32_t &)tick)) break;

		Scheduler::OnMainThread([=]() {
			sar_tas_pauseat.SetValue(tick);
		});
		return 0;
	}
	case RECV_ADVANCE_TICK: {
		Scheduler::OnMainThread([]() {
			tasPlayer->AdvanceFrame();
		});
		return 0;
	}
	case RECV_MESSAGE: {
		std::string message;

		if (!popString(cl.cmdbuf, message)) break;

		THREAD_PRINT("[TAS Protocol] %s\n", message.c_str());

		return 0;
	}
	case RECV_PLAY_SCRIPT_PROTOCOL: {

		std::string slot0Name;
		std::string slot0Script;
		std::string slot1Name;
		std::string slot1Script;

		if (!popString(cl.cmdbuf, slot0Name)) break;
		if (!popString(cl.cmdbuf, slot0Script)) break;
		if (!popString(cl.cmdbuf, slot1Name)) break;
		if (!popString(cl.cmdbuf, slot1Script)) break;

		Scheduler::OnMainThread([=]() {
			tasPlayer->PlayScript(slot0Name, slot0Script, slot1Name, slot1Script);
		});

		return 0;
	}
	case RECV_ENTITY_INFO:
	case RECV_SET_CONT_ENTITY_INFO: {
		std::string entSelector;

		if (!popString(cl.cmdbuf, entSelector)) break;

		if (packetId == RECV_SET_CONT_ENTITY_INFO) {
			cl.contInfoEntSelector = entSelector;
		} else {
			SendEntityInfo(cl, entSelector);
		}
		return 0;
	}
	default:
		// Bad command - disconnect
		return 2; 
	}

	// command hasn't been fully read - recover to the copy
	cl.cmdbuf = copy;
	return 1;
}

static bool receiveFromConnection(TasProtocol::ConnectionData &cl) {
	char buf[1024];
	int len = recv(cl.sock, buf, sizeof buf, 0);

	if (len == 0 || len == SOCKET_ERROR) {  // Connection closed or errored
		closesocket(cl.sock);
		return false;
	}

	cl.cmdbuf.insert(cl.cmdbuf.end(), std::begin(buf), std::begin(buf) + len);

	while (true) {
		int result = processCommand(cl);

		if (result == 2) { // invalid command - disconnect
			closesocket(cl.sock);
			return false;
		}
		if (result == 1) break; // not enough data - wait for more
	}

	return true;
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
	g_connections.clear();

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
		auto duration = ((std::chrono::duration<float>)(now - g_last_connection_attempt_timestamp)).count();
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

		g_last_connection_attempt_timestamp = now;
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
	encodeString(buf, scriptString);

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

void TasProtocol::SendTextMessage(std::string message) {
	std::vector<uint8_t> buf;
	buf.push_back(SEND_MESSAGE);
	encodeString(buf, message);
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

	int server_port = args.ArgC() >= 2 ? std::atoi(args[1]) : DEFAULT_TAS_SERVER_SOCKET;
	bool should_start_server = !g_running || g_server_port != server_port;

	if (should_start_server) {
		g_server_port = server_port;
		g_is_server.store(true);
	}
	
	g_conn_data_mutex.unlock();

	if (should_start_server) {
		restart();
	} else {
		console->Print("TAS server is already running on port %d.\n", server_port);
	}
}

CON_COMMAND(sar_tas_protocol_stop,
            "sar_tas_protocol_stop - stops every TAS protocol related connection.\n") {
	g_should_stop.store(true);
	g_stopped_manually = true;
}

CON_COMMAND(sar_tas_protocol_send_msg, "sar_tas_protocol_send_msg <message> - sends a message over TAS protocol.\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_tas_protocol_send_msg.ThisPtr()->m_pszHelpString);
	}
	
	TasProtocol::SendTextMessage(args[1]);
}
