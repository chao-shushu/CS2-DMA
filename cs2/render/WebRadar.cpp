#include "WebRadar.h"

#include "../utils/base64.h"
#include "../utils/Logger.h"
#include "Cheats.h"
#include "../game/MenuConfig.h"
#include "../game/AppState.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <sstream>
#include <algorithm>
#include <cstring>
#include <shared_mutex>

// ============================================================================
//  Minimal SHA-1 (RFC 3174) — only used for the WebSocket handshake key.
// ============================================================================
namespace {

struct SHA1Ctx {
	uint32_t state[5]{};
	uint64_t count = 0;
	uint8_t  buffer[64]{};

	SHA1Ctx() { reset(); }

	void reset() {
		state[0] = 0x67452301; state[1] = 0xEFCDAB89;
		state[2] = 0x98BADCFE; state[3] = 0x10325476;
		state[4] = 0xC3D2E1F0;
		count = 0;
		memset(buffer, 0, 64);
	}

	static uint32_t rol(uint32_t v, int bits) { return (v << bits) | (v >> (32 - bits)); }

	void transform(const uint8_t block[64]) {
		uint32_t w[80];
		for (int i = 0; i < 16; i++)
			w[i] = (block[i*4] << 24) | (block[i*4+1] << 16) | (block[i*4+2] << 8) | block[i*4+3];
		for (int i = 16; i < 80; i++)
			w[i] = rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

		uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
		for (int i = 0; i < 80; i++) {
			uint32_t f, k;
			if      (i < 20) { f = (b & c) | ((~b) & d);           k = 0x5A827999; }
			else if (i < 40) { f = b ^ c ^ d;                       k = 0x6ED9EBA1; }
			else if (i < 60) { f = (b & c) | (b & d) | (c & d);    k = 0x8F1BBCDC; }
			else              { f = b ^ c ^ d;                       k = 0xCA62C1D6; }
			uint32_t tmp = rol(a, 5) + f + e + k + w[i];
			e = d; d = c; c = rol(b, 30); b = a; a = tmp;
		}
		state[0] += a; state[1] += b; state[2] += c; state[3] += d; state[4] += e;
	}

	void update(const uint8_t* data, size_t len) {
		size_t idx = (size_t)(count % 64);
		count += len;
		for (size_t i = 0; i < len; i++) {
			buffer[idx++] = data[i];
			if (idx == 64) { transform(buffer); idx = 0; }
		}
	}

	void update(const std::string& s) { update((const uint8_t*)s.data(), s.size()); }

	std::string digest() {
		uint64_t bits = count * 8;
		uint8_t pad = 0x80;
		update(&pad, 1);
		pad = 0;
		while (count % 64 != 56) update(&pad, 1);
		uint8_t bits_be[8];
		for (int i = 7; i >= 0; i--) { bits_be[i] = (uint8_t)(bits & 0xFF); bits >>= 8; }
		update(bits_be, 8);

		std::string result(20, '\0');
		for (int i = 0; i < 5; i++) {
			result[i*4]   = (char)(state[i] >> 24);
			result[i*4+1] = (char)(state[i] >> 16);
			result[i*4+2] = (char)(state[i] >> 8);
			result[i*4+3] = (char)(state[i]);
		}
		return result;
	}
};

std::string sha1(const std::string& input) {
	SHA1Ctx ctx;
	ctx.update(input);
	return ctx.digest();
}

std::string base64_encode_raw(const std::string& bytes) {
	return base64::encode_into<std::string>(bytes.begin(), bytes.end());
}

static const char* WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

} // anonymous namespace

// ============================================================================
//  WebRadarServer implementation
// ============================================================================

WebRadarServer::~WebRadarServer() {
	Stop();
}

bool WebRadarServer::Start(uint16_t port) {
	if (m_running.load()) return true;

	m_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listenSock == INVALID_SOCKET) {
		LOG_ERROR("WebRadar", "socket() failed: {}", WSAGetLastError());
		return false;
	}

	int opt = 1;
	setsockopt(m_listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(m_listenSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		LOG_ERROR("WebRadar", "bind() on port {} failed: {}", port, WSAGetLastError());
		closesocket(m_listenSock);
		m_listenSock = INVALID_SOCKET;
		return false;
	}

	if (listen(m_listenSock, SOMAXCONN) == SOCKET_ERROR) {
		LOG_ERROR("WebRadar", "listen() failed: {}", WSAGetLastError());
		closesocket(m_listenSock);
		m_listenSock = INVALID_SOCKET;
		return false;
	}

	m_running.store(true);
	m_acceptThread = std::thread(&WebRadarServer::AcceptLoop, this);

	LOG_INFO("WebRadar", "WebSocket server listening on 0.0.0.0:{}", port);
	return true;
}

void WebRadarServer::Stop() {
	if (!m_running.load()) return;
	m_running.store(false);

	if (m_listenSock != INVALID_SOCKET) {
		closesocket(m_listenSock);
		m_listenSock = INVALID_SOCKET;
	}

	if (m_acceptThread.joinable())
		m_acceptThread.join();

	{
		std::lock_guard<std::mutex> lock(m_clientsMutex);
		for (auto s : m_clients) closesocket(s);
		m_clients.clear();
	}

	LOG_INFO("WebRadar", "Server stopped");
}

int WebRadarServer::GetClientCount() const {
	std::lock_guard<std::mutex> lock(m_clientsMutex);
	return (int)m_clients.size();
}

void WebRadarServer::AcceptLoop() {
	while (m_running.load()) {
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(m_listenSock, &readSet);
		timeval tv{ 0, 500000 }; // 500 ms poll

		int sel = select(0, &readSet, nullptr, nullptr, &tv);
		if (sel <= 0) continue;

		SOCKET clientSock = accept(m_listenSock, nullptr, nullptr);
		if (clientSock == INVALID_SOCKET) continue;

		// Set send timeout so Broadcast doesn't block on slow clients
		DWORD sendTimeout = 2000;
		setsockopt(clientSock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&sendTimeout, sizeof(sendTimeout));

		LOG_INFO("WebRadar", "New connection accepted");

		// Detached thread per client — lightweight; radar typically has 1–3 clients
		std::thread([this, clientSock]() {
			this->ClientLoop(clientSock);
		}).detach();
	}
}

void WebRadarServer::ClientLoop(SOCKET clientSock) {
	if (!DoHandshake(clientSock)) {
		closesocket(clientSock);
		return;
	}

	{
		std::lock_guard<std::mutex> lock(m_clientsMutex);
		m_clients.push_back(clientSock);
	}
	LOG_INFO("WebRadar", "Handshake OK, clients: {}", GetClientCount());

	// Read loop: handle ping/close from browser, ignore data frames
	while (m_running.load()) {
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(clientSock, &readSet);
		timeval tv{ 1, 0 };

		int sel = select(0, &readSet, nullptr, nullptr, &tv);
		if (sel < 0) break;
		if (sel == 0) continue;

		uint8_t header[2];
		int received = recv(clientSock, (char*)header, 2, 0);
		if (received <= 0) break;

		uint8_t opcode = header[0] & 0x0F;
		bool masked = (header[1] & 0x80) != 0;
		uint64_t payloadLen = header[1] & 0x7F;

		if (payloadLen == 126) {
			uint8_t ext[2];
			if (recv(clientSock, (char*)ext, 2, 0) != 2) break;
			payloadLen = ((uint64_t)ext[0] << 8) | ext[1];
		} else if (payloadLen == 127) {
			uint8_t ext[8];
			if (recv(clientSock, (char*)ext, 8, 0) != 8) break;
			payloadLen = 0;
			for (int i = 0; i < 8; i++) payloadLen = (payloadLen << 8) | ext[i];
		}

		uint8_t maskKey[4]{};
		if (masked && recv(clientSock, (char*)maskKey, 4, 0) != 4) break;

		// Read and discard payload (we don't process client data)
		if (payloadLen > 65536) break;
		std::string payload((size_t)payloadLen, '\0');
		size_t totalRead = 0;
		while (totalRead < payloadLen) {
			int r = recv(clientSock, &payload[totalRead], (int)(payloadLen - totalRead), 0);
			if (r <= 0) goto disconnect;
			totalRead += r;
		}

		if (opcode == 0x8) break; // Close frame

		if (opcode == 0x9) { // Ping → Pong
			uint8_t pong[2] = { 0x8A, 0x00 };
			send(clientSock, (const char*)pong, 2, 0);
		}
	}

disconnect:
	RemoveClient(clientSock);
	closesocket(clientSock);
	LOG_INFO("WebRadar", "Client disconnected, clients: {}", GetClientCount());
}

bool WebRadarServer::DoHandshake(SOCKET clientSock) {
	char buf[4096];
	int received = recv(clientSock, buf, sizeof(buf) - 1, 0);
	if (received <= 0) return false;
	buf[received] = '\0';

	std::string request(buf, received);

	LOG_DEBUG("WebRadar", "Handshake: received {} bytes", received);
	// Must be a WebSocket upgrade
	if (request.find("Upgrade:") == std::string::npos) {
		LOG_DEBUG("WebRadar", "Handshake: not a WebSocket upgrade request");
		return false;
	}

	// Extract Sec-WebSocket-Key
	auto keyPos = request.find("Sec-WebSocket-Key:");
	if (keyPos == std::string::npos) return false;
	keyPos = request.find(':', keyPos) + 1;
	while (keyPos < request.size() && request[keyPos] == ' ') keyPos++;
	auto keyEnd = request.find("\r\n", keyPos);
	if (keyEnd == std::string::npos) return false;
	std::string wsKey = request.substr(keyPos, keyEnd - keyPos);

	// Compute Sec-WebSocket-Accept per RFC 6455
	std::string acceptHash = sha1(wsKey + WS_GUID);
	std::string acceptB64 = base64_encode_raw(acceptHash);

	std::string response =
		"HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: " + acceptB64 + "\r\n"
		"\r\n";

	LOG_DEBUG("WebRadar", "Handshake: sending 101 response ({} bytes)", response.size());
	return send(clientSock, response.c_str(), (int)response.size(), 0) > 0;
}

bool WebRadarServer::SendFrame(SOCKET sock, const std::string& payload) {
	std::vector<uint8_t> frame;
	frame.reserve(10 + payload.size());

	// FIN=1, opcode=0x1 (text) — JSON payload, browser receives as string directly
	frame.push_back(0x81);

	size_t len = payload.size();
	if (len < 126) {
		frame.push_back((uint8_t)len);
	} else if (len < 65536) {
		frame.push_back(126);
		frame.push_back((uint8_t)(len >> 8));
		frame.push_back((uint8_t)(len & 0xFF));
	} else {
		frame.push_back(127);
		for (int i = 7; i >= 0; i--)
			frame.push_back((uint8_t)((len >> (i * 8)) & 0xFF));
	}

	frame.insert(frame.end(), payload.begin(), payload.end());

	int total = (int)frame.size();
	int sent = 0;
	while (sent < total) {
		int r = send(sock, (const char*)frame.data() + sent, total - sent, 0);
		if (r <= 0) return false;
		sent += r;
	}
	return true;
}

void WebRadarServer::Broadcast(const std::string& message) {
	std::lock_guard<std::mutex> lock(m_clientsMutex);
	LOG_TRACE("WebRadar", "Broadcast: {} bytes to {} clients", message.size(), m_clients.size());
	for (auto it = m_clients.begin(); it != m_clients.end(); ) {
		if (!SendFrame(*it, message)) {
			LOG_DEBUG("WebRadar", "SendFrame failed, dropping client");
			closesocket(*it);
			it = m_clients.erase(it);
		} else {
			++it;
		}
	}
}

void WebRadarServer::RemoveClient(SOCKET sock) {
	std::lock_guard<std::mutex> lock(m_clientsMutex);
	m_clients.erase(std::remove(m_clients.begin(), m_clients.end(), sock), m_clients.end());
}

// ============================================================================
//  Vite Dev Server process management (auto-start/stop with WebRadar toggle)
// ============================================================================

static HANDLE g_viteJob = nullptr;
static HANDLE g_viteProcess = nullptr;

static std::string FindWebappDir() {
	char exePath[MAX_PATH]{};
	GetModuleFileNameA(nullptr, exePath, MAX_PATH);
	std::string exeDir(exePath);
	exeDir = exeDir.substr(0, exeDir.find_last_of("\\/"));

	// Try known paths relative to exe
	const char* candidates[] = {
		"\\external\\webradar\\webapp",
		"\\webapp",
	};
	for (auto rel : candidates) {
		std::string path = exeDir + rel;
		if (GetFileAttributesA((path + "\\package.json").c_str()) != INVALID_FILE_ATTRIBUTES)
			return path;
	}
	return {};
}

static void StartViteDevServer() {
	if (g_viteProcess) return;

	std::string webappDir = FindWebappDir();
	if (webappDir.empty()) {
		LOG_ERROR("WebRadar", "Webapp directory not found, cannot start frontend");
		return;
	}

	// Job Object: kills entire process tree (cmd→npx→node) when handle is closed
	g_viteJob = CreateJobObjectA(nullptr, nullptr);
	if (g_viteJob) {
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli{};
		jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		SetInformationJobObject(g_viteJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
	}

	STARTUPINFOA si{};
	PROCESS_INFORMATION pi{};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	char cmd[] = "cmd /c npx vite --host";

	if (CreateProcessA(nullptr, cmd, nullptr, nullptr, FALSE,
		CREATE_NO_WINDOW | CREATE_SUSPENDED, nullptr, webappDir.c_str(), &si, &pi))
	{
		if (g_viteJob)
			AssignProcessToJobObject(g_viteJob, pi.hProcess);
		ResumeThread(pi.hThread);
		g_viteProcess = pi.hProcess;
		CloseHandle(pi.hThread);
		LOG_INFO("WebRadar", "Vite dev server started (PID: {}, dir: {})", pi.dwProcessId, webappDir);
	} else {
		LOG_ERROR("WebRadar", "Failed to start Vite: error {}", GetLastError());
		if (g_viteJob) { CloseHandle(g_viteJob); g_viteJob = nullptr; }
	}
}

static void StopViteDevServer() {
	if (g_viteJob) {
		TerminateJobObject(g_viteJob, 0);
		CloseHandle(g_viteJob);
		g_viteJob = nullptr;
	}
	if (g_viteProcess) {
		CloseHandle(g_viteProcess);
		g_viteProcess = nullptr;
	}
	LOG_INFO("WebRadar", "Vite dev server stopped");
}

// ============================================================================
//  JSON Serialization — converts GameSnapshot → cs2_webradar JSON format
// ============================================================================

static std::string CleanMapName(const char* raw) {
	std::string name(raw);
	if (name.find("maps/") == 0) name = name.substr(5);
	auto pos = name.find(".vpk");
	if (pos != std::string::npos) name = name.substr(0, pos);
	pos = name.find(".bsp");
	if (pos != std::string::npos) name = name.substr(0, pos);
	if (name.empty() || name.find("<empty>") != std::string::npos)
		return "invalid";
	return name;
}

// Weapon category for slot assignment (only active weapon is known via DMA)
enum class WeaponSlot { Unknown, Primary, Secondary, Melee, Utility, C4 };
static WeaponSlot CategorizeWeapon(const std::string& name) {
	if (name.empty() || name == "None" || name == "Weapon_None") return WeaponSlot::Unknown;
	// Pistols
	if (name == "glock" || name == "hkp2000" || name == "usp_silencer" || name == "p250" ||
		name == "tec9" || name == "elite" || name == "fiveseven" || name == "deagle" ||
		name == "revolver" || name == "cz75a") return WeaponSlot::Secondary;
	// Primary
	if (name == "ak47" || name == "m4a1" || name == "m4a1_silencer" || name == "aug" ||
		name == "sg556" || name == "famas" || name == "galilar" || name == "awp" ||
		name == "ssg08" || name == "scar20" || name == "g3sg1" || name == "mac10" ||
		name == "mp9" || name == "mp7" || name == "mp5sd" || name == "ump45" ||
		name == "p90" || name == "bizon" || name == "nova" || name == "xm1014" ||
		name == "sawedoff" || name == "mag7" || name == "m249" || name == "negev")
		return WeaponSlot::Primary;
	// Melee
	if (name.find("knife") != std::string::npos || name.find("bayonet") != std::string::npos ||
		name == "taser") return WeaponSlot::Melee;
	// Grenades
	if (name == "hegrenade" || name == "flashbang" || name == "smokegrenade" ||
		name == "molotov" || name == "incgrenade" || name == "decoy")
		return WeaponSlot::Utility;
	if (name == "c4") return WeaponSlot::C4;
	return WeaponSlot::Unknown;
}

// Clean weapon name: strip "weapon_" prefix artifacts and normalize
static std::string CleanWeaponName(const std::string& raw) {
	if (raw.empty() || raw == "Weapon_None" || raw == "None") return "";
	return raw;
}

// Build m_weapons JSON object from full weapon list + active weapon
static void BuildWeaponsObject(rapidjson::Value& weapons, const std::string& rawActive,
	const std::vector<std::string>& weaponList, rapidjson::Document::AllocatorType& a) {
	weapons.SetObject();
	std::string active = CleanWeaponName(rawActive);
	if (!active.empty())
		weapons.AddMember("m_active", rapidjson::Value(active.c_str(), a), a);

	// If we have a full weapon list, use it for proper categorization
	const auto& src = weaponList.empty() ? std::vector<std::string>{active} : weaponList;

	std::string primary, secondary;
	rapidjson::Value melee(rapidjson::kArrayType);
	rapidjson::Value utilities(rapidjson::kArrayType);

	for (const auto& w : src) {
		std::string name = CleanWeaponName(w);
		if (name.empty()) continue;
		WeaponSlot slot = CategorizeWeapon(name);
		switch (slot) {
		case WeaponSlot::Primary:   if (primary.empty()) primary = name; break;
		case WeaponSlot::Secondary: if (secondary.empty()) secondary = name; break;
		case WeaponSlot::Melee:     melee.PushBack(rapidjson::Value(name.c_str(), a), a); break;
		case WeaponSlot::Utility:   utilities.PushBack(rapidjson::Value(name.c_str(), a), a); break;
		default: break;
		}
	}
	if (!primary.empty())
		weapons.AddMember("m_primary", rapidjson::Value(primary.c_str(), a), a);
	if (!secondary.empty())
		weapons.AddMember("m_secondary", rapidjson::Value(secondary.c_str(), a), a);
	if (melee.Size() > 0)
		weapons.AddMember("m_melee", melee, a);
	if (utilities.Size() > 0)
		weapons.AddMember("m_utilities", utilities, a);
}

// Serialize a single player entity into JSON
static void SerializePlayer(rapidjson::Value& players, const CEntity& e,
	int idx, bool isDead, bool hasBomb, rapidjson::Document::AllocatorType& a)
{
	rapidjson::Value p(rapidjson::kObjectType);
	p.AddMember("m_idx", idx, a);
	p.AddMember("m_name", rapidjson::Value(e.Controller.PlayerName.c_str(), a), a);
	// Color: -1 means unassigned, clamp to 0-5 range for frontend (6 colors)
	int color = e.Controller.Color;
	if (color < 0 || color > 5) color = idx % 6;
	p.AddMember("m_color", color, a);
	p.AddMember("m_team", e.Controller.TeamID, a);
	p.AddMember("m_health", e.Pawn.Health, a);
	p.AddMember("m_is_dead", isDead, a);
	p.AddMember("m_armor", e.Controller.Armor, a);
	p.AddMember("m_money", e.Controller.Money, a);

	rapidjson::Value pos(rapidjson::kObjectType);
	pos.AddMember("x", e.Pawn.Pos.x, a);
	pos.AddMember("y", e.Pawn.Pos.y, a);
	p.AddMember("m_position", pos, a);

	p.AddMember("m_eye_angle", e.Pawn.ViewAngle.y, a);

	rapidjson::Value weapons(rapidjson::kObjectType);
	BuildWeaponsObject(weapons, e.Pawn.WeaponName, e.Pawn.WeaponList, a);
	p.AddMember("m_weapons", weapons, a);

	p.AddMember("m_has_helmet", e.Pawn.HasHelmet, a);
	p.AddMember("m_has_defuser", e.Pawn.HasDefuser, a);
	p.AddMember("m_has_bomb", hasBomb, a);
	p.AddMember("m_model_name", rapidjson::Value(e.Pawn.ModelName.c_str(), a), a);

	players.PushBack(p, a);
}

static std::string SerializeSnapshot(const GameSnapshot& snap) {
	rapidjson::Document doc;
	doc.SetObject();
	auto& a = doc.GetAllocator();

	doc.AddMember("m_local_team", snap.LocalPlayer.Controller.TeamID, a);

	std::string mapName = CleanMapName(snap.MapName);
	doc.AddMember("m_map", rapidjson::Value(mapName.c_str(), a), a);

	// Determine bomb carrier pawn index (lower 16 bits of entity handle)
	DWORD bombCarrierIdx = snap.Bomb.carrierPawnHandle & 0xFFFF;

	rapidjson::Value players(rapidjson::kArrayType);

	// Local player
	const auto& lp = snap.LocalPlayer;
	if (lp.Controller.TeamID >= 2) {
		bool isDead = lp.Pawn.Health <= 0;
		bool hasBomb = (!isDead && !snap.Bomb.isPlanted && bombCarrierIdx != 0 && (lp.Controller.Pawn & 0xFFFF) == bombCarrierIdx);
		SerializePlayer(players, lp, snap.LocalPlayerIndex >= 0 ? snap.LocalPlayerIndex : 999, isDead, hasBomb, a);
	}

	// Other entities — use Controller.Pawn as stable ID (React key)
	for (size_t i = 0; i < snap.Entities.size(); i++) {
		const auto& e = snap.Entities[i];
		bool isDead = e.Pawn.Health <= 0;
		bool hasBomb = (!isDead && !snap.Bomb.isPlanted && bombCarrierIdx != 0 && (e.Controller.Pawn & 0xFFFF) == bombCarrierIdx);
		SerializePlayer(players, e, (int)(e.Controller.Pawn & 0xFFFF), isDead, hasBomb, a);
	}

	doc.AddMember("m_players", players, a);

	// Bomb data
	if (snap.Bomb.isPlanted || (snap.Bomb.x != 0 || snap.Bomb.y != 0) || snap.Bomb.carrierPawnHandle != 0) {
		rapidjson::Value bomb(rapidjson::kObjectType);
		bomb.AddMember("x", snap.Bomb.x, a);
		bomb.AddMember("y", snap.Bomb.y, a);
		if (snap.Bomb.isPlanted) {
			bomb.AddMember("m_blow_time", snap.Bomb.blowTime, a);
			bomb.AddMember("m_is_defused", snap.Bomb.isDefused, a);
			bomb.AddMember("m_is_defusing", snap.Bomb.isDefusing, a);
			bomb.AddMember("m_defuse_time", snap.Bomb.defuseTime, a);
		}
		doc.AddMember("m_bomb", bomb, a);
	}

	rapidjson::StringBuffer buf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
	doc.Accept(writer);
	return buf.GetString();
}

// ============================================================================
//  WebRadarThread — lifecycle: start/stop server based on menu toggle,
//  serialize + broadcast at configurable interval.
// ============================================================================

VOID WebRadarThread() {
	LOG_INFO("WebRadar", "Thread started");

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		LOG_ERROR("WebRadar", "WSAStartup failed: {}", WSAGetLastError());
		return;
	}

	WebRadarServer server;
	bool firstBroadcast = true;

	while (true) {
		try {
			// ---- Toggle server on/off based on menu ----
			if (!MenuConfig::ShowWebRadar) {
				if (server.IsRunning()) {
					server.Stop();
					StopViteDevServer();
					LOG_INFO("WebRadar", "Disabled by user");
				}
				firstBroadcast = true;
				Sleep(500);
				continue;
			}

			if (!server.IsRunning()) {
				if (!server.Start((uint16_t)MenuConfig::WebRadarPort)) {
					LOG_ERROR("WebRadar", "Failed to start, retry in 5s");
					Sleep(5000);
					continue;
				}
				StartViteDevServer();
			}

			// ---- Wait for game ----
			if (globalVars::gameState.load() != AppState::RUNNING) {
				Sleep(200);
				continue;
			}

			// ---- Skip if no clients connected ----
			if (server.GetClientCount() == 0) {
				Sleep(100);
				continue;
			}

			// ---- Snapshot → JSON → Broadcast ----
			GameSnapshot snap;
			{
				std::shared_lock<std::shared_mutex> lock(Cheats::SnapshotMutex);
				snap = Cheats::Snapshot;
			}

			std::string json = SerializeSnapshot(snap);
			LOG_TRACE("WebRadar", "Serialized: {} bytes, map='{}', entities={}", json.size(), CleanMapName(snap.MapName), snap.Entities.size());
			server.Broadcast(json);

			if (firstBroadcast) {
				LOG_INFO("WebRadar", "First broadcast sent ({} bytes, map={}, players={})",
					json.size(), CleanMapName(snap.MapName), snap.Entities.size());
				firstBroadcast = false;
			}

			Sleep(MenuConfig::WebRadarInterval);
		}
		catch (...) {
			Sleep(1000);
		}
	}
}
