#include "NetworkGhostPlayer.hpp"

#include "DemoGhostPlayer.hpp"
#include "Event.hpp"
#include "Features/Hud/Toasts.hpp"
#include "Features/NetMessage.hpp"
#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "GhostEntity.hpp"
#include "GhostLeaderboard.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Server.hpp"
#include "Modules/SteamAPI.hpp"
#include "Modules/Surface.hpp"
#include "Scheduler.hpp"

#include <chrono>
#include <functional>
#include <queue>
#include <set>

#define ORANGE_MESSAGE_TYPE "orange-msg"

Variable ghost_sync_countdown("ghost_sync_countdown", "3", 0, "The number of seconds of countdown to show at the start of every synced map. 0 to disable.\n");
Variable ghost_spec_see_spectators("ghost_spec_see_spectators", "0", "Whether to see other spectators while spectating.\n");
Variable ghost_show_spec_chat("ghost_show_spec_chat", "1", "Show chat messages from spectators when not spectating.\n");
Variable ghost_volume("ghost_volume", "1.0", 0.f, 1.f, "Voice chat volume multiplier.\n");

#define DrawTxtRightAlign(font, x, y, clr, ...)               \
	do {                                                         \
		int _txtwidth = surface->GetFontLength(font, __VA_ARGS__);  \
		surface->DrawTxt(font, x - _txtwidth, y, clr, __VA_ARGS__); \
	} while (0)

class SyncUi {
public:
	bool active = false;
	std::optional<std::chrono::time_point<std::chrono::steady_clock>> countdownEnd;
	std::vector<uint32_t> ready;
	std::vector<uint32_t> waiting;

	SyncUi() {}

	void StartCountdown() {
		if (!this->active) return;
		if (this->countdownEnd) return;
		if (ghost_sync_countdown.GetFloat() == 0) {
			this->active = false;
			if (engine->shouldPauseForSync) {
				engine->shouldPauseForSync = false;
			} else {
				if (!engine->IsCoop() || !engine->IsOrange()) {
					engine->ExecuteCommand("unpause");
				}
				ghostLeaderboard.SyncReady();
			}
		} else {
			this->countdownEnd = NOW_STEADY() + std::chrono::milliseconds((int)(ghost_sync_countdown.GetFloat() * 1000));
		}
	}

	void UpdateAndPaint() {
		auto now = NOW_STEADY();

		if (this->countdownEnd && now >= *this->countdownEnd) {
			this->countdownEnd = {};
			this->active = false;
			if (!engine->IsCoop() || !engine->IsOrange()) {
				engine->ExecuteCommand("unpause");
			}
			ghostLeaderboard.SyncReady();
		}

		if (!this->active) return;

		int screenWidth, screenHeight;
		engine->GetScreenSize(nullptr, screenWidth, screenHeight);

		surface->DrawRect(Color{0, 0, 0, 192}, 0, 0, screenWidth, screenHeight);

		Surface::HFont font = 6;
		Color white{255, 255, 255, 255};
		Color grey{200, 200, 200, 255};

		if (this->countdownEnd) {
			unsigned ms = std::chrono::duration_cast<std::chrono::milliseconds>(*this->countdownEnd - now).count();
			if (ms >= 1000) {
				unsigned secs = (ms + 999) / 1000;  // poor man's ceil
				int length = surface->GetFontLength(font, "%d", secs);
				surface->DrawTxt(font, (screenWidth - length) / 2, 100, white, "%d", secs);
			} else {
				float secs = (float)ms / 1000.0f + 0.049f;  // poor man's ceil, but weirder
				int length = surface->GetFontLength(font, "%.1f", secs);
				surface->DrawTxt(font, (screenWidth - length) / 2, 100, white, "%.1f", secs);
			}
		} else {
			int height = surface->GetFontHeight(font);

			{
				int y = 100;
				DrawTxtRightAlign(font, screenWidth / 2 - 20, y, white, "Waiting for:");
				y += height + 15;

				for (uint32_t id : this->waiting) {
					auto ghost = networkManager.GetGhostByID(id);
					if (ghost) {
						DrawTxtRightAlign(font, screenWidth / 2 - 20, y, grey, ghost->name.c_str());
						y += height + 5;
					}
				}
			}

			{
				int y = 100;
				surface->DrawTxt(font, screenWidth / 2 + 20, y, white, "Ready:");
				y += height + 15;

				for (uint32_t id : this->ready) {
					auto ghost = networkManager.GetGhostByID(id);
					if (ghost) {
						surface->DrawTxt(font, screenWidth / 2 + 20, y, grey, ghost->name.c_str());
						y += height + 5;
					}
				}
			}
		}
	}
};

SyncUi syncUi;

Variable ghost_list_x("ghost_list_x", "2", "X position of ghost list HUD.\n", 0);
Variable ghost_list_y("ghost_list_y", "-2", "Y position of ghost list HUD.\n", 0);
Variable ghost_list_mode("ghost_list_mode", "0", 0, 2, "Mode for ghost list HUD. 0 = all players, 1 = current map, 2 = dip\n");
Variable ghost_list_show_map("ghost_list_show_map", "0", "Show the map name in the ghost list HUD.\n");
Variable ghost_list_font("ghost_list_font", "0", 0, "Font index for ghost list HUD.\n");

class PlayerListUi : public Hud {
public:
	bool active = false;

	PlayerListUi()
		: Hud(HudType_InGame | HudType_Menu | HudType_Paused | HudType_LoadingScreen, true) {
	}

	virtual bool ShouldDraw() override {
		return this->active && Hud::ShouldDraw();
	}

	virtual bool GetCurrentSize(int &w, int &h) override {
		return false;
	}

	virtual void Paint(int slot) override {
		if (slot != 0 && !engine->IsOrange()) return;
		if (!networkManager.isConnected) return;

		// HACKHACK: STUPIDEST JANK IVE EVER JANKED
		// for deep dip. free to revert when event ends.
		std::vector<std::pair<float, std::string>> players;
		networkManager.ghostPoolLock.lock();
		const static float TOWER_BOTTOM_Z = -13103.97f;
		const static float TOWER_TOP_Z = 9632.03f;
		if (ghost_list_mode.GetInt() == 2) {
			if (!networkManager.spectator) {
				auto player = client->GetPlayer(GET_SLOT() + 1);
				if (player) {
					float percent = std::clamp((client->GetAbsOrigin((void *)player).z + 64.0f - TOWER_BOTTOM_Z) / (TOWER_TOP_Z - TOWER_BOTTOM_Z), 0.0f, 1.0f);
					players.push_back({percent, Utils::ssprintf("%s (%.2f%%)", networkManager.name.c_str(), percent * 100.0f)});
				}
			}
		} else {
			if (ghost_list_show_map.GetBool()) {
				players.push_back({0, Utils::ssprintf("%s (%s)", networkManager.name.c_str(), engine->GetCurrentMapTitle().c_str())});
			} else {
				players.push_back({0, networkManager.name});
			}
		}
		for (auto &g : networkManager.ghostPool) {
			if (g->isDestroyed) continue;
			if (!networkManager.AcknowledgeGhost(g)) continue;
			if (ghost_list_mode.GetInt() >= 1 && !g->sameMap) continue;
			if (ghost_list_mode.GetInt() == 2) {
				if (g->newPos.position.z == 0.0f) continue;
				float percent = std::clamp((g->newPos.position.z + 64.0f - TOWER_BOTTOM_Z) / (TOWER_TOP_Z - TOWER_BOTTOM_Z), 0.0f, 1.0f);
				players.push_back({percent, Utils::ssprintf("%s (%.2f%%)", g->name.c_str(), percent * 100.0f)});
			} else {
				if (ghost_list_show_map.GetBool()) {
					players.push_back({0, Utils::ssprintf("%s (%s)", g->name.c_str(), engine->GetMapTitle(g->currentMap).c_str())});
				} else {
					players.push_back({0, g->name});
				}
			}
		}
		networkManager.ghostPoolLock.unlock();

		long font = scheme->GetFontByID(ghost_list_font.GetInt());

		int width = 0;
		// sort players by percentage descending
		std::sort(players.begin(), players.end(), [](const auto &a, const auto &b) {
			return a.first > b.first;
		});
		for (auto &p : players) {
			int w = surface->GetFontLength(font, "%s", p.second.c_str());
			if (w > width) width = w;
		}
		width += 6;  // Padding

		int height = players.size() * (3 + surface->GetFontHeight(font)) + 3;

		int sw, sh;
		engine->GetScreenSize(nullptr, sw, sh);

		int x = ghost_list_x.GetInt() < 0 ? sw - width + ghost_list_x.GetInt() : ghost_list_x.GetInt();
		int y = ghost_list_y.GetInt() < 0 ? sh - height + ghost_list_y.GetInt() : ghost_list_y.GetInt();

		surface->DrawRect({0, 0, 0, 192}, x, y, x + width, y + height);

		x += 3;
		y += 3;

		for (auto &p : players) {
			surface->DrawTxt(font, x, y, {255, 255, 255, 255}, "%s", p.second.c_str());
			y += surface->GetFontHeight(font) + 3;
		}
	}
};

PlayerListUi playerListUi;

Command ghost_list_on("+ghost_list", +[](const CCommand &args) { playerListUi.active = true; }, "+ghost_list - enable the ghost list HUD\n");
Command ghost_list_off("-ghost_list", +[](const CCommand &args) { playerListUi.active = false; }, "-ghost_list - disable the ghost list HUD\n");

static bool syncPauseDone = true;

ON_EVENT(PRE_TICK) {
	if (engine->IsGamePaused()) {
		syncPauseDone = true;
	}

	if (!engine->IsGamePaused() && syncPauseDone) {
		syncUi.active = false;
		syncUi.countdownEnd = {};
	}
}

// DataGhost

sf::Packet &operator>>(sf::Packet &packet, QAngle &angle) {
	return packet >> angle.x >> angle.y >> angle.z;
}

sf::Packet &operator<<(sf::Packet &packet, const QAngle &angle) {
	return packet << angle.x << angle.y << angle.z;
}

sf::Packet &operator>>(sf::Packet &packet, Vector &vec) {
	return packet >> vec.x >> vec.y >> vec.z;
}

sf::Packet &operator<<(sf::Packet &packet, const Vector &vec) {
	return packet << vec.x << vec.y << vec.z;
}

// The view offset is packed into 7 bits; this is fine as it should
// never exceed 64
sf::Packet &operator>>(sf::Packet &packet, DataGhost &dataGhost) {
	uint8_t data;
	auto &ret = packet >> dataGhost.position >> dataGhost.view_angle >> data;
	dataGhost.view_offset = (float)(data & 0x7F);
	dataGhost.grounded = (data & 0x80) != 0;
	return ret;
}
sf::Packet &operator<<(sf::Packet &packet, const DataGhost &dataGhost) {
	uint8_t data = ((int)dataGhost.view_offset & 0x7F) | (dataGhost.grounded ? 0x80 : 0x00);
	return packet << dataGhost.position << dataGhost.view_angle << data;
}

// HEADER

sf::Packet &operator>>(sf::Packet &packet, HEADER &header) {
	uint8_t tmp;
	packet >> tmp;
	header = static_cast<HEADER>(tmp);
	return packet;
}

sf::Packet &operator<<(sf::Packet &packet, const HEADER &header) {
	return packet << static_cast<uint8_t>(header);
}

// Color (RGB only, no alpha!)

sf::Packet &operator>>(sf::Packet &packet, Color &col) {
	col.a = 255;
	return packet >> col.r >> col.g >> col.b;
}

sf::Packet &operator<<(sf::Packet &packet, const Color &col) {
	return packet << col.r << col.g << col.b;
}

Variable ghost_TCP_only("ghost_TCP_only", "0", "Uses only TCP for ghost servers. For people with unreliable internet.\n");
Variable ghost_update_rate("ghost_update_rate", "50", 1, "Milliseconds between ghost updates. For people with slow/metered internet.\n");
Variable ghost_net_dump("ghost_net_dump", "0", "Dump all ghost network activity to a file for debugging.\n");

static FILE *g_dumpFile;
static float g_dumpBaseTime;

static void startNetDump() {
	g_dumpFile = fopen("ghost_net_dump.csv", "w");
	if (g_dumpFile) {
		fputs("Time,Type,Info\n", g_dumpFile);
		g_dumpBaseTime = engine->engineTool->Original<float(__rescall *)(void *thisptr)>(Offsets::HostTick - 1)(engine->engineTool->ThisPtr());
	}
}

static void endNetDump() {
	if (!g_dumpFile) return;
	fclose(g_dumpFile);
	g_dumpFile = nullptr;
}

static void addToNetDump(const char *type, const char *info) {
	if (!g_dumpFile) return;
	float time = engine->engineTool->Original<float(__rescall *)(void *thisptr)>(Offsets::HostTick - 1)(engine->engineTool->ThisPtr()) - g_dumpBaseTime;
	fprintf(g_dumpFile, "%.2f,%s,%s\n", time, type, info ? info : "");
}

CON_COMMAND(ghost_net_dump_mark, "Mark a point of interest in the ghost network activity dump.\n") {
	addToNetDump("mark", nullptr);
}

std::mutex mutex;

NetworkManager networkManager;

NetworkManager::NetworkManager()
	: serverIP(sf::IpAddress::LocalHost)
	, serverPort(53000)
	, name("")
	, isCountdownReady(false)
	, modelName("models/props/food_can/food_can_open.mdl")
	, spectator(false) {
}

void NetworkManager::Connect(sf::IpAddress ip, unsigned short int port, bool spectator) {
	std::thread connectionThread([this, ip, port, spectator]() {
		if (this->tcpSocket.connect(ip, port, sf::seconds(5)) != sf::Socket::Status::Done) {
			toastHud.AddToast(GHOST_TOAST_TAG, Utils::ssprintf("Connection timed out! Cannot connect to the server at %s:%d", ip.toString().c_str(), port));
			return;
		}

		if (this->udpSocket.bind(sf::Socket::AnyPort) != sf::Socket::Status::Done) {
			toastHud.AddToast(GHOST_TOAST_TAG, Utils::ssprintf("Connection timed out! Cannot connect to the server at %s:%d", ip.toString().c_str(), port));
			return;
		}

		this->udpSocket.setBlocking(false);
		this->tcpSocket.setBlocking(true);

		this->serverIP = ip;
		this->serverPort = port;

		this->spectator = spectator;

		sf::Packet connection_packet;
		connection_packet << HEADER::CONNECT << this->udpSocket.getLocalPort() << this->name.c_str() << DataGhost{{0, 0, 0}, {0, 0, 0}, 0, false} << this->modelName.c_str() << engine->GetCurrentMapName().c_str() << ghost_TCP_only.GetBool() << GhostEntity::set_color << spectator;
		this->tcpSocket.send(connection_packet);

		{
			sf::SocketSelector tcpSelector;
			tcpSelector.add(this->tcpSocket);

			sf::Packet confirm_connection;
			if (!tcpSelector.wait(sf::seconds(30))) {
				toastHud.AddToast(GHOST_TOAST_TAG, Utils::ssprintf("Handshake timed out! Cannot connect to the server at %s:%d", ip.toString().c_str(), port));
				return;
			}

			if (this->tcpSocket.receive(confirm_connection) != sf::Socket::Status::Done) {
				toastHud.AddToast(GHOST_TOAST_TAG, Utils::ssprintf("Transfer timed out! Cannot connect to the server at %s:%d", ip.toString().c_str(), port));
				return;
			}

			//	Get our ID
			confirm_connection >> this->ID;

			if (!this->spectator) ghostLeaderboard.AddNew(this->ID, this->name);

			//	Add every player connected to the ghostPool
			int nb_players = 0;
			int nb_spectators = 0;
			uint32_t nb_ghosts;
			confirm_connection >> nb_ghosts;
			for (uint32_t i = 0; i < nb_ghosts; ++i) {
				uint32_t ID;
				std::string name;
				DataGhost data;
				std::string model_name;
				std::string current_map;
				Color color;
				bool spectator;
				confirm_connection >> ID >> name >> data >> model_name >> current_map >> color >> spectator;

				auto ghost = std::make_shared<GhostEntity>(ID, name, data, current_map, true);
				ghost->modelName = model_name;
				ghost->color = color;
				ghost->spectator = spectator;
				this->ghostPoolLock.lock();
				this->ghostPool.push_back(ghost);
				this->ghostPoolLock.unlock();
				if (!spectator) ghostLeaderboard.AddNew(ghost->ID, ghost->name);
				if (spectator)
					++nb_spectators;
				else
					++nb_players;
			}

			this->UpdateGhostsSameMap();
			if (engine->isRunning()) {
				this->SpawnAllGhosts();
			}

			if (this->spectator) {
				toastHud.AddToast(GHOST_TOAST_TAG, Utils::ssprintf("Successfully connected to the server as a spectator!\n%d players and %d other spectators connected\n", nb_players, nb_spectators));
			} else {
				toastHud.AddToast(GHOST_TOAST_TAG, Utils::ssprintf("Successfully connected to the server!\n%d other players connected\n", nb_players));
			}
		}  //	End of the scope. Will kill the Selector

		this->isConnected = true;
		this->runThread = true;
		this->waitForRunning.notify_one();
		this->networkThread = std::thread(&NetworkManager::RunNetwork, this);
		this->networkThread.detach();

		if (ghost_net_dump.GetBool()) {
			startNetDump();
			addToNetDump("connect", Utils::ssprintf("%s:%d", ip.toString().c_str(), port).c_str());
		}
	});
	connectionThread.detach();
}

void NetworkManager::Disconnect() {
	if (this->isConnected) {
		addToNetDump("disconnect", nullptr);
		endNetDump();

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

		Scheduler::OnMainThread([=]() {
			toastHud.AddToast(GHOST_TOAST_TAG, "You have been disconnected");
		});
	}
}

void NetworkManager::StopServer() {
	this->Disconnect();
}

void NetworkManager::PauseNetwork() {
	this->runThread = false;
}

void NetworkManager::ResumeNetwork() {
	this->runThread = true;
	this->waitForRunning.notify_one();
}

void NetworkManager::RunNetwork() {
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

			if ((engine->isRunning() || engine->IsOrange()) && !engine->IsGamePaused()) {
				this->SendPlayerData();
			}

			this->lastUpdateTime = now;
		}

		if (this->selector.wait(sf::milliseconds(ghost_update_rate.GetInt()))) {
			if (this->selector.isReady(this->udpSocket)) {  //	UDP
				std::vector<sf::Packet> buffer;
				this->ReceiveUDPUpdates(buffer);
				for (auto &packet : buffer) {
					this->Treat(packet, true);
				}
			}

			if (this->selector.isReady(this->tcpSocket)) {  //	TCP
				sf::Packet packet;
				sf::Socket::Status status;
				status = this->tcpSocket.receive(packet);
				if (status != sf::Socket::Status::Done) {
					if (status == sf::Socket::Status::Disconnected) {  //	If connection with the server lost (crash for e.g.)
						this->Disconnect();
						break;
					}
					continue;
				}
				this->Treat(packet, false);
			}
		}
	}
}

void NetworkManager::SendPlayerData() {
	{
		sf::Packet packet;
		packet << HEADER::UPDATE << this->ID;
		auto player = client->GetPlayer(GET_SLOT() + 1);
		if (player) {
			bool grounded = player->ground_entity();
			packet << DataGhost{client->GetAbsOrigin(player), engine->GetAngles(engine->IsOrange() ? 0 : GET_SLOT()), client->GetViewOffset(player).z, grounded};
		} else {
			packet << DataGhost::Invalid();
		}

		if (!ghost_TCP_only.GetBool()) {
			this->udpSocket.send(packet, this->serverIP, this->serverPort);
		} else {
			this->tcpSocket.send(packet);
		}
	}

	// voice.
	{
		// read local microphone input.
		uint32_t nBytesAvailable = 0;
		EVoiceResult res = steam->SteamUser()->GetAvailableVoice(&nBytesAvailable, NULL, 0);

		if (res == k_EVoiceResultOK && nBytesAvailable > 0) {
			uint32_t nBytesWritten = 0;
			MsgVoiceChatData_t msg;

			// don't send more than 1 KB at a time.
			uint8_t buffer[1024 + sizeof(msg)];

			res = steam->SteamUser()->GetVoice(true, buffer + sizeof(msg), 1024, &nBytesWritten, false, NULL, 0, NULL, 0);

			if (res == k_EVoiceResultOK && nBytesWritten > 0) {
				// add voicedata to start of msg.
				msg.SetDataLength(nBytesWritten);
				memcpy(buffer, &msg, sizeof(msg));

				sf::Packet packet;
				packet << HEADER::VOICE << this->ID;
				packet.append(buffer, sizeof(msg) + nBytesWritten);

				if (!ghost_TCP_only.GetBool()) {
					this->udpSocket.send(packet, this->serverIP, this->serverPort);
				} else {
					this->tcpSocket.send(packet);
				}
			}
		}
	}
}

void NetworkManager::NotifyMapChange() {
	sf::Packet packet;

	if (ghost_show_advancement.GetInt() >= 3 && AcknowledgeGhost(nullptr)) {
		std::string msg;
		if (this->splitTicks != (uint32_t)-1) {
			auto ipt = engine->GetIPT();
			std::string time = SpeedrunTimer::Format(this->splitTicks * ipt);
			std::string totalTime = SpeedrunTimer::Format(this->splitTicksTotal * ipt);
			if (this->splitTicks < this->splitTicksTotal) {
				msg = Utils::ssprintf("%s is now on %s (%s -> %s)", this->name.c_str(), engine->GetCurrentMapTitle().c_str(), time.c_str(), totalTime.c_str());
			} else {
				msg = Utils::ssprintf("%s is now on %s (%s)", this->name.c_str(), engine->GetCurrentMapTitle().c_str(), totalTime.c_str());
			}
		} else {
			msg = Utils::ssprintf("%s is now on %s", this->name.c_str(), engine->GetCurrentMapTitle().c_str());
		}
		toastHud.AddToast(GHOST_TOAST_TAG, msg);
	}

	addToNetDump("send-map-change", engine->GetCurrentMapName().c_str());
	ghostLeaderboard.GhostLoad(this->ID, this->splitTicksTotal, ghost_sync.GetBool());

	packet << HEADER::MAP_CHANGE << this->ID << engine->GetCurrentMapName().c_str() << this->splitTicks << this->splitTicksTotal;
	this->tcpSocket.send(packet);
}

void NetworkManager::NotifySpeedrunFinished(const bool CM) {
	sf::Packet packet;
	packet << HEADER::SPEEDRUN_FINISH << this->ID;

	float totalSecs = 0;
	auto ipt = engine->GetIPT();

	if (CM) {
		totalSecs = server->GetCMTimer();
	} else {
		totalSecs = SpeedrunTimer::GetTotalTicks() * ipt;
	}

	std::string time = SpeedrunTimer::Format(totalSecs);

	if (ghost_show_advancement.GetInt() >= 1 && AcknowledgeGhost(nullptr)) toastHud.AddToast(GHOST_TOAST_TAG, Utils::ssprintf("%s has finished on %s in %s", this->name.c_str(), engine->GetCurrentMapTitle().c_str(), time.c_str()));
	ghostLeaderboard.GhostFinished(this->ID, (int)roundf(totalSecs / ipt));

	addToNetDump("send-speedrun-finish", time.c_str());

	packet << time.c_str();

	this->tcpSocket.send(packet);
}

void NetworkManager::SendMessageToAll(std::string msg) {
	addToNetDump("send-message", msg.c_str());
	sf::Packet packet;
	packet << HEADER::MESSAGE << this->ID << msg.c_str();
	this->tcpSocket.send(packet);
	std::string name = this->name;
	if (this->spectator) name += " (spectator)";
	this->PrintMessage(name.c_str(), GhostEntity::set_color, msg);
}

Color NetworkManager::AdjustGhostColorForChat(Color col) {
	col.a = 255;
	if (col.r == 0 && col.g == 0 && col.b == 0) {
		// Black is the default ghost color. Override it with a slight grey, since black looks really bad in chat
		col = {192, 192, 192};
	}
	return col;
}

void NetworkManager::PrintMessage(const char *sender, Color sender_col, const std::string &message) {
	sender_col = AdjustGhostColorForChat(sender_col);

	std::vector<std::pair<Color, std::string>> components;

	std::string name_comp = Utils::ssprintf("%s: ", sender ? sender : "SERVER");
	components.push_back({sender ? sender_col : Color{255, 50, 40}, name_comp});

	Color def_col = sender ? Color{255, 255, 255} : Color{255, 100, 100};

	// find ghost names in message
	// this is slow as all shit
	bool was_last_alnum = false;
	size_t comp_start = 0;
	for (size_t i = 0; i < message.size(); ++i) {
		auto isAlnum = [](char c) {
			if (c >= 'a' && c <= 'z') return true;
			if (c >= 'A' && c <= 'Z') return true;
			if (c >= '0' && c <= '9') return true;
			return false;
		};

		if (!was_last_alnum) {
			// check for own name first
			// also check trailing char isn't alnum
			char post = i + this->name.size() >= message.size() ? 0 : message[i + this->name.size()];
			if (!isAlnum(post) && Utils::StartsWithInsens(message.c_str() + i, this->name.c_str())) {
				if (i > comp_start) {
					components.push_back({def_col, message.substr(comp_start, i - comp_start)});
				}
				auto col = AdjustGhostColorForChat(GhostEntity::set_color);
				components.push_back({col, message.substr(i, this->name.size())});
				comp_start = i + this->name.size();
				i = comp_start - 1;
			} else {
				// check for other ghost names
				this->ghostPoolLock.lock();
				for (auto other : this->ghostPool) {
					// also check trailing char isn't alnum
					post = i + other->name.size() >= message.size() ? 0 : message[i + other->name.size()];
					if (!isAlnum(post) && Utils::StartsWithInsens(message.c_str() + i, other->name.c_str())) {
						if (i > comp_start) {
							components.push_back({def_col, message.substr(comp_start, i - comp_start)});
						}
						auto col = AdjustGhostColorForChat(other->GetColor());
						components.push_back({col, message.substr(i, other->name.size())});
						comp_start = i + other->name.size();
						i = comp_start - 1;
						break;
					}
				}
				this->ghostPoolLock.unlock();
			}
		}

		was_last_alnum = isAlnum(message[i]);
	}
	if (comp_start != message.size()) {
		components.push_back({def_col, message.substr(comp_start)});
	}

	client->MultiColorChat(components);
}

void NetworkManager::SendPing() {
	addToNetDump("send-ping", nullptr);
	sf::Packet packet;
	packet << HEADER::PING << this->ID;
	this->tcpSocket.send(packet);
	this->pingClock.restart();
}

void NetworkManager::ReceiveUDPUpdates(std::vector<sf::Packet> &buffer) {
	sf::Socket::Status status;
	do {
		sf::Packet packet;
		std::optional<sf::IpAddress> ip;
		unsigned short int port;
		status = this->udpSocket.receive(packet, ip, port);
		if (status == sf::Socket::Status::Done) {
			buffer.push_back(packet);
		}
	} while (status == sf::Socket::Status::Done);
}

void NetworkManager::Treat(sf::Packet &packet, bool udp) {
	HEADER header;
	uint32_t ID;
	packet >> header >> ID;

	switch (header) {
	case HEADER::NONE:
		break;
	case HEADER::PING: {
		auto ping = this->pingClock.getElapsedTime();
		addToNetDump("recv-ping", Utils::ssprintf("%d;%d", ID, ping.asMilliseconds()).c_str());
		Scheduler::OnMainThread([=]() {
			console->Print("Ping: %d ms\n", ping.asMilliseconds());
		});
		break;
	}
	case HEADER::CONNECT: {
		std::string name;
		DataGhost data;
		std::string model_name;
		std::string current_map;
		Color col;
		bool spectator;
		packet >> name >> data >> model_name >> current_map >> col >> spectator;
		auto ghost = std::make_shared<GhostEntity>(ID, name, data, current_map, true);
		ghost->modelName = model_name;
		ghost->color = col;
		ghost->spectator = spectator;

		addToNetDump("recv-connect", Utils::ssprintf("%d;%s;%s", ID, name.c_str(), current_map.c_str()).c_str());

		this->ghostPoolLock.lock();
		this->ghostPool.push_back(ghost);
		this->ghostPoolLock.unlock();

		Scheduler::OnMainThread([=]() {
			if (this->AcknowledgeGhost(ghost)) {
				if (!strcmp("", current_map.c_str())) {
					toastHud.AddToast(GHOST_TOAST_TAG, Utils::ssprintf("%s%s has connected in the menu!", name.c_str(), ghost->spectator ? " (spectator)" : ""));
				} else {
					toastHud.AddToast(GHOST_TOAST_TAG, Utils::ssprintf("%s%s has connected in %s!", name.c_str(), ghost->spectator ? " (spectator)" : "", engine->GetMapTitle(current_map).c_str()));
				}
			}

			if (!spectator) ghostLeaderboard.AddNew(ghost->ID, ghost->name);

			this->UpdateGhostsSameMap();
			if (this->AcknowledgeGhost(ghost)) {
				if (ghost->sameMap && engine->isRunning()) {
					ghost->Spawn();
				}
			}
		});

		break;
	}
	case HEADER::DISCONNECT: {
		addToNetDump("recv-disconnect", Utils::ssprintf("%d", ID).c_str());
		this->ghostPoolLock.lock();
		int toErase = -1;
		for (size_t i = 0; i < this->ghostPool.size(); ++i) {
			if (this->ghostPool[i]->ID == ID) {
				auto ghost = this->ghostPool[i];
				Scheduler::OnMainThread([=]() {
					if (this->AcknowledgeGhost(ghost)) {
						toastHud.AddToast(GHOST_TOAST_TAG, Utils::ssprintf("%s%s has disconnected!", ghost->name.c_str(), ghost->spectator ? " (spectator)" : ""));
					}
					ghost->DeleteGhost();
				});
				this->ghostPool[i]->isDestroyed = true;
				toErase = i;
				break;
			}
		}
		if (toErase != -1)
			this->ghostPool.erase(this->ghostPool.begin() + toErase);
		this->ghostPoolLock.unlock();

		Scheduler::OnMainThread([=]() {
			if (ghost_sync.GetBool()) {
				if (this->AreAllGhostsAheadOrSameMap()) {
					syncUi.StartCountdown();
				}
			}
		});
		break;
	}
	case HEADER::STOP_SERVER:
		addToNetDump("recv-stop-server", Utils::ssprintf("%d", ID).c_str());
		this->StopServer();
		break;
	case HEADER::MAP_CHANGE: {
		auto ghost = this->GetGhostByID(ID);
		if (ghost) {
			std::string map;
			uint32_t ticksIL, ticksTotal;
			packet >> map >> ticksIL >> ticksTotal;
			auto old_map = ghost->currentMap;
			ghost->currentMap = map;
			addToNetDump("recv-map-change", Utils::ssprintf("%d;%s", ID, map.c_str()).c_str());

			Scheduler::OnMainThread([=]() {
				if (ghost->isDestroyed)
					return;  // FIXME: this probably works in practice, but it isn't entirely thread-safe

				this->UpdateGhostsSameMap();
				if (ghost_show_advancement.GetInt() >= 3 && this->AcknowledgeGhost(ghost)) {
					if (ticksIL == (uint32_t)-1) {
						std::string msg = Utils::ssprintf("%s is now on %s", ghost->name.c_str(), engine->GetMapTitle(ghost->currentMap).c_str());
						toastHud.AddToast(GHOST_TOAST_TAG, msg);
					} else {
						auto ipt = engine->GetIPT();
						std::string time = SpeedrunTimer::Format(ticksIL * ipt);
						std::string timeTotal = SpeedrunTimer::Format(ticksTotal * ipt);
						std::string msg = Utils::ssprintf("%s is now on %s (%s -> %s)", ghost->name.c_str(), engine->GetMapTitle(ghost->currentMap).c_str(), time.c_str(), timeTotal.c_str());
						toastHud.AddToast(GHOST_TOAST_TAG, msg);
					}
				}

				if (old_map != map) ghostLeaderboard.GhostLoad(ID, ticksTotal, ghost_sync.GetBool());

				if (ghost->sameMap && this->AcknowledgeGhost(ghost)) {
					ghost->Spawn();
				} else {
					ghost->DeleteGhost();
				}

				if (ghost->IsBeingFollowed()) {
					auto cmd = Utils::ssprintf("changelevel %s", ghost->currentMap.c_str());
					engine->ExecuteCommand(cmd.c_str());
				}

				if (ghost_sync.GetBool()) {
					if (this->AreAllGhostsAheadOrSameMap()) {
						syncUi.StartCountdown();
					}
				}
			});
		}
		break;
	}
	case HEADER::HEART_BEAT: {
		uint32_t token;
		packet >> token;
		addToNetDump("recv-heartbeat", Utils::ssprintf("%d;%s;%X", ID, udp ? "UDP" : "TCP", token).c_str());
		sf::Packet response;
		response << HEADER::HEART_BEAT << this->ID << token;
		if (udp) {
			addToNetDump("send-heartbeat", Utils::ssprintf("UDP;%X", token).c_str());
			this->udpSocket.send(response, this->serverIP, this->serverPort);
		} else {
			addToNetDump("send-heartbeat", Utils::ssprintf("TCP;%X", token).c_str());
			this->tcpSocket.send(response);
		}
		break;
	}
	case HEADER::MESSAGE: {
		std::string message;
		packet >> message;
		addToNetDump("recv-message", Utils::ssprintf("%d;%s", ID, message.c_str()).c_str());
		auto ghost = ID != 0 ? this->GetGhostByID(ID) : nullptr;
		if (ghost || ID == 0) {
			Scheduler::OnMainThread([=]() {
				if (ID == 0 || ghost_show_spec_chat.GetBool() || this->AcknowledgeGhost(ghost)) {
					Color col = ghost ? ghost->GetColor() : Color{0, 0, 0};
					std::string name = ghost ? ghost->name : "";
					if (ghost && ghost->spectator) name += " (spectator)";
					this->PrintMessage(ID == 0 ? nullptr : name.c_str(), col, message);
				}
			});
		}
		break;
	}
	case HEADER::COUNTDOWN: {
		uint8_t step;
		packet >> step;
		addToNetDump("recv-countdown", Utils::ssprintf("%d;%d", ID, (int)step).c_str());
		if (step == 0) {  //	Countdown setup
			std::string preCommands;
			std::string postCommands;
			uint32_t duration;
			packet >> duration >> preCommands >> postCommands;

			this->SetupCountdown(preCommands, postCommands, duration);

			sf::Packet confirm_packet;
			confirm_packet << HEADER::COUNTDOWN << this->ID << uint8_t(1);
			addToNetDump("send-countdown", "1");
			this->tcpSocket.send(confirm_packet);
		} else if (step == 1) {  //	Exec
			this->StartCountdown();
		}
		break;
	}
	case HEADER::SPEEDRUN_FINISH: {
		std::string timer;
		packet >> timer;
		auto ghost = this->GetGhostByID(ID);
		addToNetDump("recv-speedrun-finish", Utils::ssprintf("%d;%s", ID, timer.c_str()).c_str());
		if (ghost) {
			Scheduler::OnMainThread([=]() {
				if (ghost_show_advancement.GetInt() >= 2 || (ghost->sameMap && ghost_show_advancement.GetInt() >= 1)) {
					toastHud.AddToast(GHOST_TOAST_TAG, Utils::ssprintf("%s has finished on %s in %s", ghost->name.c_str(), engine->GetMapTitle(ghost->currentMap).c_str(), timer.c_str()));
				}
				// whose fucking idea was it to send a string?!
				float totalSecs = SpeedrunTimer::UnFormat(timer);
				auto ipt = engine->GetIPT();
				ghostLeaderboard.GhostFinished(ID, (int)roundf(totalSecs / ipt));
			});
		}
		break;
	}
	case HEADER::MODEL_CHANGE: {
		std::string modelName;
		packet >> modelName;
		auto ghost = this->GetGhostByID(ID);
		addToNetDump("recv-model-change", Utils::ssprintf("%d;%s", ID, modelName.c_str()).c_str());
		if (ghost) {
			ghost->modelName = modelName;
			Scheduler::OnMainThread([=]() {
				if (ghost->isDestroyed)
					return;  // FIXME: this probably works in practice, but it isn't entirely thread-safe
				if (ghost->sameMap && engine->isRunning()) {
					ghost->DeleteGhost();
					if (this->AcknowledgeGhost(ghost)) ghost->Spawn();
				}
			});
		}
		break;
	}
	case HEADER::COLOR_CHANGE: {
		Color col;
		packet >> col;
		auto ghost = this->GetGhostByID(ID);
		addToNetDump("recv-color-change", Utils::ssprintf("%d;%02X%02X%02X", ID, col.r, col.g, col.b).c_str());
		if (ghost) ghost->color = col;
		break;
	}
	case HEADER::UPDATE: {
		if (ID == 0) {
			// This packet contains updates for multiple ghosts
			uint32_t nupdates;
			packet >> nupdates;
			for (size_t i = 0; i < nupdates; ++i) {
				uint32_t ghost_id;
				DataGhost data;
				packet >> ghost_id >> data;

				if (ghost_id == this->ID) continue;
				if (!data.IsValid()) continue;
				auto ghost = this->GetGhostByID(ghost_id);
				if (!ghost) continue;

				ghost->SetData(data, true);
			}
		}
		break;
	}
	case HEADER::TAUNT: {
		std::string animName;
		packet >> animName;
		auto ghost = this->GetGhostByID(ID);
		addToNetDump("recv-taunt", Utils::ssprintf("%d;%s", ID, animName.c_str()).c_str());
		if (ghost) {
			Scheduler::OnMainThread([=]() {
				if (this->AcknowledgeGhost(ghost)) {
					auto it = std::find_if(
						g_ghostTauntAnimationDefinitions.begin(),
						g_ghostTauntAnimationDefinitions.end(),
						[&](const GhostAnimationDefinition &def) { return def.name == animName; });
					if (it != g_ghostTauntAnimationDefinitions.end()) {
						ghost->renderer.StartAnimation(*it);
					}
				}
			});
		}
		break;
	}
	case HEADER::LOCATOR: {
		Vector position;
		Vector normal;
		packet >> position >> normal;
		auto ghost = this->GetGhostByID(ID);
		addToNetDump("recv-locator", Utils::ssprintf("%d;%.1f,%.1f,%.1f;%.1f,%.1f,%.1f", ID, position.x, position.y, position.z, normal.x, normal.y, normal.z).c_str());
		if (ghost && engine->GetCurrentMapName() == ghost->currentMap) {
			Scheduler::OnMainThread([=]() {
				if (this->AcknowledgeGhost(ghost)) {
					client->ShowLocator(position, normal, ghost->color.value_or(Color(255,255,255)));
				}
			});
		}
		break;
	}
	case HEADER::VOICE: {
		// check if on same map.
		auto ghost = this->GetGhostByID(ID);
		if (engine->GetCurrentMapName() != ghost->currentMap)
			break;

		// skip first 5 bytes (header, id).
		const auto pMessage = (uintptr_t)(packet.getData()) + 5;

		// get data from start of buffer.
		const MsgVoiceChatData_t *pMsgVoiceData = (const MsgVoiceChatData_t *)pMessage;

		constexpr uint32_t sampleRate = 11025;  // 44100 for highest qual, but this is funnier c:
		uint8_t pbUncompressedVoice[sampleRate];
		uint32_t numUncompressedBytes = 0;

		// skip past data header.
		const uint8_t *pVoiceData = (const uint8_t *)pMessage;
		pVoiceData += sizeof(MsgVoiceChatData_t);

		// decompress.
		EVoiceResult res = steam->SteamUser()->DecompressVoice(pVoiceData, pMsgVoiceData->GetDataLength(), pbUncompressedVoice, sizeof(pbUncompressedVoice), &numUncompressedBytes, sampleRate);

		// continuous stream per id for voice.
		static std::unordered_map<uint32_t, std::unique_ptr<VoiceStream>> voiceStreams;
		if (!voiceStreams.count(ghost->ID))
			voiceStreams.insert(std::make_pair(ghost->ID, new VoiceStream(sampleRate)));

		if (res == k_EVoiceResultOK && numUncompressedBytes > 0) {
			auto stream = voiceStreams[ghost->ID].get();

			// load from raw pcm data.
			stream->pushSamples((const int16_t *)pbUncompressedVoice, numUncompressedBytes / sizeof(int16_t));

			// account for ingame vol.
			static auto vol = Variable("volume");
			stream->setVolume(vol.GetFloat() * ghost_volume.GetFloat() * 10000.f);

			// proximity.
			auto player = client->GetPlayer(GET_SLOT() + 1);
			if (player) {
				auto origin = client->GetAbsOrigin(player) / 128.f;
				auto angles = client->GetAbsAngles(player);
				auto ghost_pos = ghost->data.position / 128.f;

				Vector forward;
				Math::AngleVectors(angles, &forward);

				sf::Listener::setPosition({origin.x, origin.z, origin.y});
				sf::Listener::setDirection({-forward.x, -forward.z, -forward.y});

				stream->setPosition({ghost_pos.x, ghost_pos.z, ghost_pos.y});
			}

			if (stream->getStatus() != sf::SoundSource::Status::Playing)
				stream->play();
		}

		break;
	}
	default:
		break;
	}
}

void NetworkManager::UpdateGhostsPosition() {
	// Copy the pool since rendering via Lerp tries to lock the pool
	// further down in processing
	this->ghostPoolLock.lock();
	auto pool_copy(this->ghostPool);
	this->ghostPoolLock.unlock();

	for (auto ghost : pool_copy) {
		if (ghost->sameMap && this->AcknowledgeGhost(ghost)) {
			ghost->Lerp();
		}
	}
}

std::shared_ptr<GhostEntity> NetworkManager::GetGhostByID(uint32_t ID) {
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

void NetworkManager::UpdateGhostsSameMap() {
	int mapIdx = engine->GetMapIndex(engine->GetCurrentMapName());
	this->ghostPoolLock.lock();
	for (auto ghost : this->ghostPool) {
		ghost->sameMap = strcmp(ghost->currentMap.c_str(), "") && ghost->currentMap == engine->GetCurrentMapName();
		if (mapIdx == -1)
			ghost->isAhead = false;  // Fallback - unknown map
		else
			ghost->isAhead = engine->GetMapIndex(ghost->currentMap) > mapIdx;
	}
	this->ghostPoolLock.unlock();
}

void NetworkManager::UpdateModel(const std::string modelName) {
	this->modelName = modelName;
	if (this->isConnected) {
		sf::Packet packet;
		addToNetDump("send-model-change", modelName.c_str());
		packet << HEADER::MODEL_CHANGE << this->ID << this->modelName.c_str();
		this->tcpSocket.send(packet);
	}
}

void NetworkManager::UpdateColor() {
	if (!this->isConnected) return;
	Color col = GhostEntity::set_color;
	addToNetDump("send-color-change", Utils::ssprintf("%02X%02X%02X", col.r, col.g, col.b).c_str());
	sf::Packet packet;
	packet << HEADER::COLOR_CHANGE << this->ID << col;
	this->tcpSocket.send(packet);
}

void NetworkManager::NotifyTaunt(const std::string name) {
	if (!this->isConnected) return;
	addToNetDump("send-taunt", name.c_str());
	sf::Packet packet;
	packet << HEADER::TAUNT << this->ID << name.c_str();
	this->tcpSocket.send(packet);
}

void NetworkManager::NotifyLocator(Vector position, Vector normal) {
	if (!this->isConnected) return;
	addToNetDump("send-locator", Utils::ssprintf("%d;%.1f,%.1f,%.1f;%.1f,%.1f,%.1f", this->ID, position.x, position.y, position.z, normal.x, normal.y, normal.z).c_str());
	sf::Packet packet;
	packet << HEADER::LOCATOR << this->ID << position << normal;
	this->tcpSocket.send(packet);
}

bool NetworkManager::AreAllGhostsAheadOrSameMap() {
	this->ghostPoolLock.lock();
	syncUi.ready.clear();
	syncUi.waiting.clear();
	bool allReady = true;
	for (auto ghost : this->ghostPool) {
		if (ghost->spectator) continue;
		if (!ghost->isAhead && !ghost->sameMap) {
			syncUi.waiting.push_back(ghost->ID);
			allReady = false;
		} else {
			syncUi.ready.push_back(ghost->ID);
		}
	}
	this->ghostPoolLock.unlock();

	return allReady;
}

void NetworkManager::SpawnAllGhosts() {
	this->ghostPoolLock.lock();
	for (auto ghost : this->ghostPool) {
		if (ghost->sameMap) {
			if (this->AcknowledgeGhost(ghost)) {
				ghost->Spawn();
			}
		}
	}
	this->ghostPoolLock.unlock();
}

void NetworkManager::DeleteAllGhosts() {
	this->ghostPoolLock.lock();
	for (auto ghost : this->ghostPool) {
		ghost->DeleteGhost();
	}
	this->ghostPoolLock.unlock();
}

void NetworkManager::SetupCountdown(std::string preCommands, std::string postCommands, uint32_t duration) {
	Scheduler::OnMainThread([=]() {
		engine->ExecuteCommand(preCommands.c_str());
	});
	this->postCountdownCommands = postCommands;
	this->countdownStep = duration;
	this->countdownShow = duration != 0;

	this->timeLeft = NOW_STEADY();
}

void NetworkManager::StartCountdown() {
	auto ping = std::chrono::duration_cast<std::chrono::milliseconds>(NOW_STEADY() - this->timeLeft);
	this->timeLeft = NOW_STEADY() + std::chrono::seconds(3) - ping;
	this->isCountdownReady = true;
}

void NetworkManager::UpdateCountdown() {
	auto now = NOW_STEADY();
	if (std::chrono::duration_cast<std::chrono::milliseconds>(now - this->timeLeft).count() >= 1000) {
		if (this->countdownStep == 0) {
			if (this->countdownShow) {
				client->Chat({255, 80, 70}, "0! GO!");
			}
			if (!this->postCountdownCommands.empty()) {
				engine->ExecuteCommand(this->postCountdownCommands.c_str());
			}
			this->isCountdownReady = false;
		} else {
			client->Chat({255, 80, 70}, Utils::ssprintf("%d...", this->countdownStep).c_str());
		}
		this->countdownStep--;
		this->timeLeft = now;
	}
}

bool NetworkManager::IsSyncing() {
	return this->isConnected.load() && syncUi.active;
}

bool NetworkManager::AcknowledgeGhost(std::shared_ptr<GhostEntity> ghost) {
	bool spec = ghost ? ghost->spectator : this->spectator;
	if (!spec) return true;
	return this->spectator && ghost_spec_see_spectators.GetBool();
}

void NetworkManager::UpdateSyncUi() {
	syncUi.UpdateAndPaint();
}

ON_EVENT(RENDER) {
	if (networkManager.isConnected && (engine->isRunning() || engine->IsOrange())) {
		// Is this executed on orange?? (is the engine considered running?)
		networkManager.UpdateGhostsPosition();
	}
}

ON_EVENT(PRE_TICK) {
	if (networkManager.isConnected && (engine->isRunning() || engine->IsOrange())) {
		if (networkManager.isCountdownReady) {
			networkManager.UpdateCountdown();
		}
	}
}

ON_EVENT(SESSION_START) {
	if (networkManager.isConnected) {
		networkManager.NotifyMapChange();
		networkManager.UpdateGhostsSameMap();
		networkManager.SpawnAllGhosts();
		syncPauseDone = false;
		if (ghost_sync.GetBool()) {
			if (networkManager.disableSyncForLoad) {
				networkManager.disableSyncForLoad = false;
			} else {
				if (session->previousMap != engine->GetCurrentMapName()) {  //	Don't pause if just reloading save
					engine->shouldPauseForSync = true;
					syncUi.active = true;
					syncUi.countdownEnd = {};
					if (networkManager.AreAllGhostsAheadOrSameMap()) syncUi.StartCountdown();
				}
			}
		}
	}
}

// Commands

CON_COMMAND(ghost_connect,
            "ghost_connect <ip address> <port> - connect to the server\n"
            "ex: 'localhost 53000' - '127.0.0.1 53000' - '89.10.20.20 53000'.\n") {
	if (args.ArgC() < 2 || args.ArgC() > 3) {
		return console->Print(ghost_connect.ThisPtr()->m_pszHelpString);
	}

	if (networkManager.name == "") {
		networkManager.name = Variable("name").GetString();
	}

	if (networkManager.isConnected) {
		return console->Print("You must disconnect from your current ghost server before connecting to another.\n");
	}

	networkManager.Connect(*sf::IpAddress::resolve(args[1]), args.ArgC() >= 3 ? std::atoi(args[2]) : 53000, false);
}

CON_COMMAND(ghost_spec_connect,
            "ghost_spec_connect <ip address> <port> - connect to the server as a spectator\n"
            "ex: 'localhost 53000' - '127.0.0.1 53000' - '89.10.20.20 53000'.\n") {
	if (args.ArgC() < 2 || args.ArgC() > 3) {
		return console->Print(ghost_spec_connect.ThisPtr()->m_pszHelpString);
	}

	if (networkManager.name == "") {
		networkManager.name = Variable("name").GetString();
	}

	if (networkManager.isConnected) {
		return console->Print("You must disconnect from your current ghost server before connecting to another.\n");
	}

	networkManager.Connect(*sf::IpAddress::resolve(args[1]), args.ArgC() >= 3 ? std::atoi(args[2]) : 53000, true);
}

CON_COMMAND(ghost_disconnect, "ghost_disconnect - disconnect\n") {
	networkManager.Disconnect();
}

CON_COMMAND(ghost_name, "ghost_name - change your online name\n") {
	if (networkManager.isConnected) {
		return console->Print("Cannot change name while connected to a server.\n");
	}

	networkManager.name = args[1];
}

CON_COMMAND(ghost_message, "ghost_message - send message to other players\n") {
	if (args.ArgC() < 2) {
		return console->Print(ghost_message.ThisPtr()->m_pszHelpString);
	}

	std::string msg = args[1];
	for (int i = 2; i < args.ArgC(); ++i) {
		msg += " " + std::string(args[i]);
	}

	networkManager.SendMessageToAll(msg);
}

CON_COMMAND(ghost_ping, "Pong!\n") {
	networkManager.SendPing();
}

int g_chatType = 0;
int g_wasChatType = 0;

bool NetworkManager::HandleGhostSay(const char *msg, int clientidx) {
	if (Utils::StartsWith(msg, "\x07")) return false;  // orange saying IT'S REAL
	if (clientidx != 1) {
		if (g_partnerHasSAR) {
			NetMessage::SendMsg(ORANGE_MESSAGE_TYPE, msg, strlen(msg));
			return true;
		}
		return false;
	}

	if (g_chatType != 0 || g_wasChatType != 2) {
		return false;
	}

	if (networkManager.isConnected) {
		networkManager.SendMessageToAll(msg);
	}

	g_wasChatType = 0;
	return true;
}

ON_INIT {
	NetMessage::RegisterHandler(ORANGE_MESSAGE_TYPE, +[](const void *data, size_t size) {
		auto msg = std::string((char *)data, size);
		if (engine->IsOrange()) {
			if (g_wasChatType == 2) {
				if (networkManager.isConnected) networkManager.SendMessageToAll(msg.c_str());
			} else {
				engine->ExecuteCommand(Utils::ssprintf("say \"\x07%s\"", msg.c_str()).c_str(), true);
			}
			g_wasChatType = 0;
		} });
}

ON_EVENT(FRAME) {
	if ((g_chatType == 2 && !networkManager.isConnected) || (g_chatType == 0 && cl_chat_active.GetBool())) g_chatType = 1;

	if (g_chatType != 0) {
		g_wasChatType = g_chatType;
	}
	if (!cl_chat_active.GetBool()) {
		g_chatType = 0;
	}
}

Command ghost_voice_on("+ghost_voice", +[](const CCommand &args) { steam->SteamUser()->StartVoiceRecording(); }, "+ghost_voice - push to talk in voice chat\n");
Command ghost_voice_off("-ghost_voice", +[](const CCommand &args) { steam->SteamUser()->StopVoiceRecording(); }, "-ghost_voice - push to talk in voice chat\n");

CON_COMMAND(ghost_chat, "ghost_chat - open the chat HUD for messaging other players\n") {
	if (g_chatType == 0 && networkManager.isConnected) {
		g_wasChatType = 0;
		g_chatType = 2;
		client->OpenChat();
	}
}

CON_COMMAND(ghost_debug, "ghost_debug - output a fuckton of debug info about network ghosts\n") {
	if (!networkManager.isConnected) {
		return console->Print("Not connected to a server\n");
	}

	console->Print("Connected to %s:%hu with name \"%s\" and id 0x%02X\n", networkManager.serverIP.toString().c_str(), networkManager.serverPort, networkManager.name.c_str(), networkManager.ID);

	console->Print("Current ghost pool:\n");

	networkManager.ghostPoolLock.lock();
	for (size_t i = 0; i < networkManager.ghostPool.size(); ++i) {
		auto ghost = networkManager.ghostPool[i];
		console->Print("  [0x%02X] 0x%02X: \"%s\" on \"%s\" (%s)", i, ghost->ID, ghost->name.c_str(), ghost->currentMap.c_str(), ghost->sameMap ? "same map" : ghost->isAhead ? "ahead"
		                                                                                                                                                                      : "behind");
		if (ghost->isDestroyed)
			console->Print(" [DESTROYED]\n");
		else
			console->Print("\n");
	}
	networkManager.ghostPoolLock.unlock();
}

CON_COMMAND(ghost_list, "ghost_list - list all players in the current ghost server\n") {
	if (!networkManager.isConnected) {
		return console->Print("Not connected to a server\n");
	}

	networkManager.ghostPoolLock.lock();
	console->Print("%d ghosts connected:\n", networkManager.ghostPool.size());
	for (size_t i = 0; i < networkManager.ghostPool.size(); ++i) {
		auto ghost = networkManager.ghostPool[i];
		if (!ghost->isDestroyed) {
			console->Print("  %s (%s)%s\n", ghost->name.c_str(), ghost->currentMap.size() == 0 ? "menu" : engine->GetMapTitle(ghost->currentMap).c_str(), ghost->spectator ? " (spectator)" : "");
		}
	}
	networkManager.ghostPoolLock.unlock();
}
