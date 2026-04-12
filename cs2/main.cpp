#include "config/SettingsManager.h"

#include "config/Language.h"
#include "game/AppState.h"

#include "utils/Logger.h"
#include "utils/CrashHandler.h"

#ifndef NTSTATUS
typedef long NTSTATUS;
#endif

#include "game/Threads.h"
#include "game/MenuConfig.h"
#include "render/GrenadeHelper.h"
#include "config/ConfigSaver.h"

#include <iostream>
#include <filesystem>
#include <windows.h>
#include <timeapi.h>
#include <winhttp.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "winhttp.lib")


namespace fs = std::filesystem;

std::string readFile(const std::string& path) {
	std::ifstream file(path);
	if (!file) return "";
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

static std::string downloadUrl(const wchar_t* host, const wchar_t* path) {
	std::string result;
	HINTERNET hSession = WinHttpOpen(L"CS2-DMA/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) return result;

	HINTERNET hConnect = WinHttpConnect(hSession, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
	if (!hConnect) { WinHttpCloseHandle(hSession); return result; }

	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path, NULL,
		WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return result; }

	DWORD timeout = 5000;
	WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
	WinHttpSetOption(hRequest, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

	if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
		WinHttpReceiveResponse(hRequest, NULL)) {
		DWORD statusCode = 0, size = sizeof(statusCode);
		WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
			NULL, &statusCode, &size, NULL);
		if (statusCode == 200) {
			char buffer[4096];
			DWORD bytesRead = 0;
			while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
				result.append(buffer, bytesRead);
				bytesRead = 0;
			}
		}
	}

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);
	return result;
}


void main(HMODULE module) {
	timeBeginPeriod(1);
	Logger::Get().Init("logs");
	CrashHandler::Install("logs");

	LOG_INFO("DMA", "CS2-DMA starting...");
	LOG_INFO("DMA", "Software coded by 枯潮");

	settingsJson.LoadSettings();
	LOG_INFO("Config", "Settings parsed (language: {})", settingsJson.language);

	if (settingsJson.language == "en") lang.english();
	else if (settingsJson.language == "ch") lang.chineese();
	else lang.chineese();

	std::string offsets = readFile("data/offsets.json");
	std::string client = readFile("data/client_dll.json");

	LOG_INFO("Config", "Checking offsets against GitHub repository...");
	std::string remoteOffsets = downloadUrl(L"raw.githubusercontent.com", L"/chao-shushu/CS2-DMA/main/data/offsets.json");
	std::string remoteClient = downloadUrl(L"raw.githubusercontent.com", L"/chao-shushu/CS2-DMA/main/data/client_dll.json");

	if (!remoteOffsets.empty() && !remoteClient.empty()) {
		if (offsets != remoteOffsets || client != remoteClient) {
			std::cout << "\n========================================" << std::endl;
			std::cout << "\xc6\xab\xd2\xc6\xd6\xb5\xd0\xa3\xd1\xe9\xce\xb4\xcd\xa8\xb9\xfd\xa3\xac\xc7\xeb\xc8\xb7\xc8\xcf\xca\xc7\xb7\xf1\xca\xc7\xd7\xee\xd0\xc2\xc6\xab\xd2\xc6\xd6\xb5\xa3\xbf" << std::endl;
			std::cout << "Offset verification failed. Are you using the latest offsets?" << std::endl;
			std::cout << "GitHub: https://github.com/chao-shushu/CS2-DMA/tree/main/data" << std::endl;
			std::cout << "========================================\n" << std::endl;
			std::cout << "\xbc\xcc\xd0\xf8\xca\xb9\xd3\xc3\xb1\xbe\xb5\xd8\xc6\xab\xd2\xc6\xd6\xb5? / Continue with local offsets? (y/n): ";
			char choice = 'n';
			std::cin >> choice;
			if (choice != 'y' && choice != 'Y') {
				LOG_INFO("Config", "User declined to use local offsets, exiting");
				return;
			}
			LOG_INFO("Config", "User confirmed to use local offsets");
		} else {
			LOG_INFO("Config", "Local offsets match GitHub repository");
		}
	} else {
		LOG_WARNING("Config", "Could not fetch remote offsets, skipping validation");
	}

	Offset::UpdateOffsets(offsets, client);
	LOG_INFO("Config", "Offsets updated");

	if (!fs::directory_entry(MenuConfig::path).exists()) {
		fs::create_directory(MenuConfig::path);
		LOG_INFO("Config", "Created config folder: {}", MenuConfig::path);
	}

	MyConfigSaver::LoadConfig("_autosave.config");
	if (MenuConfig::SelectedLanguage == 0) lang.english();
	else lang.chineese();
	LOG_INFO("Config", "Auto-loaded settings from _autosave.config");

	GrenadeHelper::LoadMapData("data/grenade-helper");
	LOG_INFO("DMA", "Grenade helper loaded");

	globalVars::gameState.store(AppState::DMA_INITIALIZING);
	LOG_INFO("DMA", "Initializing DMA device...");

	if (!ProcessMgr.InitDMA()) {
		globalVars::gameState.store(AppState::DMA_FAILED);
		LOG_ERROR("DMA", "DMA connection failed!");
	} else {
		LOG_INFO("DMA", "DMA connected successfully");
		ProcessMgr.init_keystates();
		globalVars::gameState.store(AppState::SEARCHING_GAME);
	}

	auto safeCreateThread = [](LPTHREAD_START_ROUTINE threadFunc, const char* name) -> bool {
		HANDLE hThread = CreateThread(nullptr, 0, threadFunc, NULL, 0, 0);
		if (hThread == NULL) {
			LOG_ERROR("DMA", "Failed to create {} thread (error: {})", name, (unsigned long)GetLastError());
			return false;
		}
		CloseHandle(hThread);
		return true;
	};

	if (globalVars::gameState.load() != AppState::DMA_FAILED) {
		safeCreateThread((LPTHREAD_START_ROUTINE)(ConnectionThread), "ConnectionThread");
		safeCreateThread((LPTHREAD_START_ROUTINE)(DataThread), "DataThread");
		safeCreateThread((LPTHREAD_START_ROUTINE)(SlowUpdateThread), "SlowUpdateThread");
		safeCreateThread((LPTHREAD_START_ROUTINE)(KeysCheckThread), "KeysCheckThread");
		safeCreateThread((LPTHREAD_START_ROUTINE)(WebRadarThread), "WebRadarThread");
		LOG_INFO("DMA", "All threads started, searching for cs2.exe...");
	}

	SetThreadPriority(GetCurrentThread(), HIGH_PRIORITY_CLASS);

	Gui.NewWindow("CS2DMA", Vec2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)), Cheats::Run);
}