#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <cstdint>

// Lightweight WebSocket server for Web Radar (RFC 6455)
// Zero external dependencies — uses only Winsock2 (already linked).
// Binds to 0.0.0.0 so any LAN device can connect.
class WebRadarServer {
public:
	WebRadarServer() = default;
	~WebRadarServer();

	WebRadarServer(const WebRadarServer&) = delete;
	WebRadarServer& operator=(const WebRadarServer&) = delete;

	bool Start(uint16_t port);
	void Stop();

	// Send a message to all connected WebSocket clients.
	// Thread-safe — can be called from any thread.
	void Broadcast(const std::string& message);

	bool IsRunning() const { return m_running.load(); }
	int  GetClientCount() const;

private:
	void AcceptLoop();
	void ClientLoop(SOCKET clientSock);
	bool DoHandshake(SOCKET clientSock);
	bool DoHandshakeWithRequest(SOCKET clientSock, const std::string& request);
	bool SendFrame(SOCKET sock, const std::string& payload);
	void RemoveClient(SOCKET sock);

	SOCKET m_listenSock = INVALID_SOCKET;
	std::atomic<bool> m_running{ false };
	std::thread m_acceptThread;

	mutable std::mutex m_clientsMutex;
	std::vector<SOCKET> m_clients;
};

// Thread function — reads GameSnapshot, serializes to JSON, broadcasts via WebSocket.
// Declared here, implemented in WebRadar.cpp, started from main.cpp alongside other threads.
VOID WebRadarThread();
