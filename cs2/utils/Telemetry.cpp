#include "Telemetry.h"

#ifdef BETA_TELEMETRY

#include "Logger.h"
#include "base64.h"

#include <string>
#include <fstream>
#include <sstream>
#include <mutex>
#include <chrono>
#include <cstring>
#include <cstdio>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

// =====================================================================
//  PAT token — loaded from TelemetryToken.h (gitignored, not in repo)
// =====================================================================
#include "TelemetryToken.h"

// =====================================================================
//  Internal state
// =====================================================================
static std::mutex  g_TelMutex;
static std::string g_LogFilePath;
static std::string g_CrashLogPath;
static std::string g_CrashDmpPath;
static bool        g_Initialized = false;

// =====================================================================
//  GitHub repo config
// =====================================================================
static const wchar_t* GH_API_HOST  = L"api.github.com";
static const char*    GH_REPO      = "chao-shushu/card";
static const char*    GH_BRANCH    = "main";

// =====================================================================
//  Helpers
// =====================================================================
static std::string GetComputerName()
{
    char buf[256] = {};
    DWORD size = sizeof(buf);
    if (GetComputerNameA(buf, &size))
        return std::string(buf);
    return "unknown";
}

static std::string MakeRemotePath(const std::string& filename)
{
    // telemetry/YYYY/MM/DD/filename
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    struct tm ti{};
    localtime_s(&ti, &t);

    char path[256];
    snprintf(path, sizeof(path), "telemetry/%04d/%02d/%02d/%s",
             ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday, filename.c_str());
    return path;
}

static std::string ExtractFilename(const std::string& path)
{
    size_t pos = path.find_last_of("/\\");
    return (pos != std::string::npos) ? path.substr(pos + 1) : path;
}

static bool ReadFileContent(const std::string& path, std::string& out)
{
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return true;
}

// =====================================================================
//  GitHub Contents API upload (PUT)
// =====================================================================
static bool UploadToGitHub(const std::string& remotePath, const std::string& filename,
                           const std::string& contentBase64)
{
    // Build JSON body: {"message":"...","branch":"main","content":"base64..."}
    std::string commitMsg = "telemetry: upload " + filename;
    std::string body = "{\"message\":\"" + commitMsg + "\","
                       "\"branch\":\"" + GH_BRANCH + "\","
                       "\"content\":\"" + contentBase64 + "\"}";

    // Build API path: /repos/{owner}/{repo}/contents/{path}
    std::string apiPathStr = "/repos/" + std::string(GH_REPO) + "/contents/" + remotePath;
    std::wstring apiPath(apiPathStr.begin(), apiPathStr.end());

    // Build Authorization header
    std::string authStr = "token " + std::string(GH_TOKEN);
    std::wstring authHeader(authStr.begin(), authStr.end());

    HINTERNET hSession = WinHttpOpen(L"CS2-DMA-Telemetry/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        LOG_ERROR("Telemetry", "WinHttpOpen failed: {}", (unsigned long)GetLastError());
        return false;
    }

    HINTERNET hConnect = WinHttpConnect(hSession, GH_API_HOST,
                                        INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        LOG_ERROR("Telemetry", "WinHttpConnect failed: {}", (unsigned long)GetLastError());
        return false;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"PUT", apiPath.c_str(),
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG_ERROR("Telemetry", "WinHttpOpenRequest failed: {}", (unsigned long)GetLastError());
        return false;
    }

    // Timeouts
    DWORD connectTimeout = 5000;
    DWORD receiveTimeout = 10000;
    WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT, &connectTimeout, sizeof(connectTimeout));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_RECEIVE_TIMEOUT, &receiveTimeout, sizeof(receiveTimeout));

    // Set headers
    std::wstring contentType = L"application/json";
    WinHttpAddRequestHeaders(hRequest, L"Authorization", (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);
    // Set full Authorization header value
    std::wstring authLine = L"Authorization: " + authHeader;
    WinHttpAddRequestHeaders(hRequest, authLine.c_str(), (DWORD)authLine.size(), WINHTTP_ADDREQ_FLAG_ADD);

    // Send request with body
    BOOL ok = WinHttpSendRequest(hRequest,
                                 L"Content-Type: application/json",
                                 (DWORD)-1,
                                 (LPVOID)body.c_str(),
                                 (DWORD)body.size(),
                                 (DWORD)body.size(),
                                 0);
    if (!ok) {
        DWORD err = GetLastError();
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG_ERROR("Telemetry", "WinHttpSendRequest failed: {}", (unsigned long)err);
        return false;
    }

    // Receive response
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG_ERROR("Telemetry", "WinHttpReceiveResponse failed: {}", (unsigned long)GetLastError());
        return false;
    }

    // Check status code
    DWORD statusCode = 0, size = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        NULL, &statusCode, &size, NULL);

    // Read response body (for logging)
    std::string response;
    {
        char buffer[4096];
        DWORD bytesRead = 0;
        while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            response.append(buffer, bytesRead);
            bytesRead = 0;
        }
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if (statusCode == 201 || statusCode == 200) {
        LOG_INFO("Telemetry", "Uploaded {} -> {} (HTTP {})", filename, remotePath, (int)statusCode);
        return true;
    } else {
        LOG_ERROR("Telemetry", "Upload failed for {} (HTTP {})", filename, (int)statusCode);
        // Log first 200 chars of response for debugging
        if (response.size() > 200) response.resize(200);
        LOG_ERROR("Telemetry", "Response: {}", response);
        return false;
    }
}

// Upload a local file to GitHub
static bool UploadFile(const std::string& localPath)
{
    std::string content;
    if (!ReadFileContent(localPath, content)) {
        LOG_ERROR("Telemetry", "Cannot read file: {}", localPath);
        return false;
    }

    // GitHub API requires base64-encoded content
    std::string b64 = base64::to_base64(std::string_view(content));

    std::string filename = ExtractFilename(localPath);
    std::string remotePath = MakeRemotePath(filename);

    return UploadToGitHub(remotePath, filename, b64);
}

// =====================================================================
//  Public API
// =====================================================================
void Telemetry::Init()
{
    std::lock_guard<std::mutex> lock(g_TelMutex);
    g_Initialized = true;
    LOG_INFO("Telemetry", "Beta telemetry enabled (repo: {})", GH_REPO);
}

void Telemetry::SetLogFilePath(const std::string& path)
{
    std::lock_guard<std::mutex> lock(g_TelMutex);
    g_LogFilePath = path;
}

void Telemetry::SetCrashFiles(const std::string& logPath, const std::string& dmpPath)
{
    std::lock_guard<std::mutex> lock(g_TelMutex);
    g_CrashLogPath = logPath;
    g_CrashDmpPath = dmpPath;
}

void Telemetry::UploadSessionLog()
{
    std::lock_guard<std::mutex> lock(g_TelMutex);
    if (!g_Initialized || g_LogFilePath.empty()) return;

    LOG_INFO("Telemetry", "Uploading session log: {}", g_LogFilePath);
    UploadFile(g_LogFilePath);
}

void Telemetry::UploadCrashFiles()
{
    std::lock_guard<std::mutex> lock(g_TelMutex);
    if (!g_Initialized) return;

    if (!g_CrashLogPath.empty()) {
        LOG_INFO("Telemetry", "Uploading crash log: {}", g_CrashLogPath);
        UploadFile(g_CrashLogPath);
    }
    if (!g_CrashDmpPath.empty()) {
        // MiniDump files can be large (>10MB), skip if too big for GitHub API
        std::ifstream f(g_CrashDmpPath, std::ios::binary | std::ios::ate);
        if (f.is_open()) {
            auto size = f.tellg();
            if (size > 5 * 1024 * 1024) { // 5MB limit
                LOG_WARNING("Telemetry", "MiniDump too large ({}MB), skipping upload", (long long)size / (1024*1024));
            } else {
                LOG_INFO("Telemetry", "Uploading crash dump: {}", g_CrashDmpPath);
                UploadFile(g_CrashDmpPath);
            }
        }
    }
    // Also upload the session log alongside crash files
    if (!g_LogFilePath.empty()) {
        UploadFile(g_LogFilePath);
    }
}

#else // !BETA_TELEMETRY — all no-ops

void Telemetry::Init() {}
void Telemetry::SetLogFilePath(const std::string&) {}
void Telemetry::SetCrashFiles(const std::string&, const std::string&) {}
void Telemetry::UploadSessionLog() {}
void Telemetry::UploadCrashFiles() {}

#endif // BETA_TELEMETRY
