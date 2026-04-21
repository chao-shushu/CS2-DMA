#include "Logger.h"
#include "Telemetry.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <filesystem>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace fs = std::filesystem;

// =====================================================================
//  Singleton
// =====================================================================
Logger& Logger::Get()
{
    static Logger instance;
    return instance;
}

Logger::~Logger()
{
    Shutdown();
}

// =====================================================================
//  Debug mode
// =====================================================================
void Logger::SetDebugMode(bool enabled)
{
    s_DebugMode.store(enabled, std::memory_order_relaxed);
}

bool Logger::IsDebugMode()
{
    return s_DebugMode.load(std::memory_order_relaxed);
}

// =====================================================================
//  Lifecycle
// =====================================================================
void Logger::Init(const std::string& logDir)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_Initialized.load())
        return;

    if (!fs::exists(logDir))
        fs::create_directories(logDir);

    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    struct tm ti{};
    localtime_s(&ti, &t);

    char filename[128];
    snprintf(filename, sizeof(filename), "%s/cs2dma_%04d%02d%02d_%02d%02d%02d.log",
             logDir.c_str(),
             ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
             ti.tm_hour, ti.tm_min, ti.tm_sec);

    m_FilePath = filename;
    m_File.open(m_FilePath, std::ios::out | std::ios::trunc);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE)
    {
        DWORD mode = 0;
        if (GetConsoleMode(hConsole, &mode))
        {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hConsole, mode);
        }
    }

    m_Initialized.store(true);

    Telemetry::SetLogFilePath(m_FilePath);

    if (m_File.is_open())
    {
        m_File << "================================================================\n";
        m_File << "  CS2-DMA Log Session\n";
        m_File << "  Started: " << FormatTimestamp() << "\n";
        m_File << "================================================================\n\n";
        m_File.flush();
    }
}

void Logger::Shutdown()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Initialized.load())
        return;

    if (m_File.is_open())
    {
        m_File << "\n================================================================\n";
        m_File << "  Session ended: " << FormatTimestamp() << "\n";
        m_File << "================================================================\n";
        m_File.close();
    }
    m_Initialized.store(false);
}

// =====================================================================
//  Core log
// =====================================================================
void Logger::Log(LogLevel level, const char* module, const char* message)
{
    if (!m_Initialized.load())
        return;

    std::lock_guard<std::mutex> lock(m_Mutex);

    std::string ts = FormatTimestamp();

    char line[768];
    snprintf(line, sizeof(line), "[%s] [%s] [%s] %s",
             ts.c_str(), LevelToString(level), module, message);

    // Write to ring buffer
    RingEntry& entry = m_Ring[m_RingHead % RING_BUFFER_SIZE];
    strncpy_s(entry.Timestamp, ts.c_str(), _TRUNCATE);
    entry.Level = level;
    strncpy_s(entry.Module, module, _TRUNCATE);
    strncpy_s(entry.Message, message, _TRUNCATE);
    m_RingHead = (m_RingHead + 1) % RING_BUFFER_SIZE;
    if (m_RingCount < RING_BUFFER_SIZE)
        m_RingCount++;

    WriteToConsole(level, line);
    WriteFile(line);
}

// =====================================================================
//  Ring buffer snapshot
// =====================================================================
Logger::RingSnapshot Logger::GetRecentLogs() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    RingSnapshot snap;
    snap.Count = m_RingCount;

    int start = 0;
    if (m_RingCount >= RING_BUFFER_SIZE)
        start = m_RingHead; // oldest entry

    for (int i = 0; i < m_RingCount; i++)
    {
        int idx = (start + i) % RING_BUFFER_SIZE;
        snap.Entries[i] = m_Ring[idx];
    }
    return snap;
}

std::string Logger::GetLogFilePath() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_FilePath;
}

// =====================================================================
//  Console output with ANSI colors
// =====================================================================
void Logger::WriteToConsole(LogLevel level, const char* formatted)
{
    int color = LevelToColor(level);
    // ANSI: \033[38;5;{color}m
    printf("\033[38;5;%dm%s\033[0m\n", color, formatted);
}

void Logger::WriteFile(const char* formatted)
{
    if (m_File.is_open())
    {
        m_File << formatted << '\n';
        m_File.flush();
    }
}

// =====================================================================
//  Helpers
// =====================================================================
std::string Logger::FormatTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                   now.time_since_epoch()) % 1000;
    struct tm ti{};
    localtime_s(&ti, &t);

    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
             ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
             ti.tm_hour, ti.tm_min, ti.tm_sec, (int)ms.count());
    return buf;
}

const char* Logger::LevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::TRACE:   return "TRACE";
    case LogLevel::DEBUG:   return "DEBUG";
    case LogLevel::INFO:    return "INFO";
    case LogLevel::WARNING: return "WARNING";
    case LogLevel::ERR:     return "ERROR";
    case LogLevel::FATAL:   return "FATAL";
    default:                return "???";
    }
}

int Logger::LevelToColor(LogLevel level)
{
    switch (level)
    {
    case LogLevel::TRACE:   return 245; // gray
    case LogLevel::DEBUG:   return 39;  // cyan
    case LogLevel::INFO:    return 82;  // green
    case LogLevel::WARNING: return 220; // yellow
    case LogLevel::ERR:     return 196; // red
    case LogLevel::FATAL:   return 160; // dark red
    default:                return 255;
    }
}

// =====================================================================
//  Format helpers
// =====================================================================
std::string LogFormat(const char* fmt)
{
    return std::string(fmt);
}

std::string LogFormatValue(const std::string& v) { return v; }
std::string LogFormatValue(const char* v)         { return v ? v : "(null)"; }
std::string LogFormatValue(bool v)                { return v ? "true" : "false"; }

std::string LogFormatValue(int v)                { return std::to_string(v); }
std::string LogFormatValue(unsigned int v)        { return std::to_string(v); }
std::string LogFormatValue(long v)               { return std::to_string(v); }
std::string LogFormatValue(unsigned long v)       { return std::to_string(v); }
std::string LogFormatValue(long long v)          { return std::to_string(v); }
std::string LogFormatValue(unsigned long long v)  { return std::to_string(v); }

std::string LogFormatValue(float v)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%.3f", v);
    return buf;
}

std::string LogFormatValue(double v)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%.3f", v);
    return buf;
}

std::string LogFormatValue(const void* v)
{
    char buf[24];
    snprintf(buf, sizeof(buf), "0x%llX", (unsigned long long)v);
    return buf;
}

std::string LogFormatHex(unsigned long long v, bool upper)
{
    char buf[24];
    snprintf(buf, sizeof(buf), upper ? "0x%llX" : "0x%llx", v);
    return buf;
}
