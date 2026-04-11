#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <array>
#include <atomic>
#include <cstdint>
#include <type_traits>

// =====================================================================
//  Log levels
// =====================================================================
enum class LogLevel : int
{
    TRACE   = 0,
    DEBUG   = 1,
    INFO    = 2,
    WARNING = 3,
    ERR     = 4,   // ERROR conflicts with Windows macro
    FATAL   = 5,
};

// =====================================================================
//  Ring buffer entry — stored for crash diagnostics
// =====================================================================
struct RingEntry
{
    char      Timestamp[24]{};
    LogLevel  Level = LogLevel::TRACE;
    char      Module[24]{};
    char      Message[512]{};
};

constexpr int RING_BUFFER_SIZE = 64;

// =====================================================================
//  Logger — singleton, thread-safe
// =====================================================================
class Logger
{
public:
    static Logger& Get();

    // Lifecycle
    void Init(const std::string& logDir = "logs");
    void Shutdown();

    // Core log function
    void Log(LogLevel level, const char* module, const char* message);

    // Ring buffer access for crash handler
    struct RingSnapshot
    {
        RingEntry Entries[RING_BUFFER_SIZE];
        int       Count = 0;
    };
    RingSnapshot GetRecentLogs() const;

    // Current log file path (for crash handler reference)
    std::string GetLogFilePath() const;

private:
    Logger() = default;
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void WriteToConsole(LogLevel level, const char* formatted);
    void WriteFile(const char* formatted);
    std::string FormatTimestamp() const;

    static const char* LevelToString(LogLevel level);
    static int LevelToColor(LogLevel level);

    mutable std::mutex  m_Mutex;
    std::ofstream       m_File;
    std::string         m_FilePath;
    std::atomic<bool>   m_Initialized{ false };

    // Ring buffer
    RingEntry           m_Ring[RING_BUFFER_SIZE]{};
    int                 m_RingHead = 0;   // next write position
    int                 m_RingCount = 0;  // total entries written (clamped to RING_BUFFER_SIZE)
};

// =====================================================================
//  Lightweight format helper (avoids <format> / fmtlib dependency)
//
//  Supports:
//    {}    — string / integer / float (auto)
//    {:X}  — uppercase hex for integers
//    {:x}  — lowercase hex for integers
// =====================================================================
std::string LogFormat(const char* fmt);

// Terminal overloads
template <typename T, typename... Args>
std::string LogFormat(const char* fmt, T&& first, Args&&... rest);

// Specialised formatters
std::string LogFormatValue(const std::string& v);
std::string LogFormatValue(const char* v);
std::string LogFormatValue(int v);
std::string LogFormatValue(unsigned int v);
std::string LogFormatValue(long v);
std::string LogFormatValue(unsigned long v);
std::string LogFormatValue(long long v);
std::string LogFormatValue(unsigned long long v);
std::string LogFormatValue(float v);
std::string LogFormatValue(double v);
std::string LogFormatValue(bool v);
std::string LogFormatValue(const void* v);

std::string LogFormatHex(unsigned long long v, bool upper);

// Implementation of variadic template
template <typename T, typename... Args>
std::string LogFormat(const char* fmt, T&& first, Args&&... rest)
{
    std::string result;
    const char* p = fmt;
    while (*p)
    {
        if (p[0] == '{')
        {
            if (p[1] == '}')
            {
                result += LogFormatValue(std::forward<T>(first));
                return result + LogFormat(p + 2, std::forward<Args>(rest)...);
            }
            if (p[1] == ':' && (p[2] == 'X' || p[2] == 'x') && p[3] == '}')
            {
                bool upper = (p[2] == 'X');
                if constexpr (std::is_arithmetic_v<std::decay_t<T>>)
                    result += LogFormatHex(static_cast<unsigned long long>(first), upper);
                else
                    result += LogFormatValue(std::forward<T>(first));
                return result + LogFormat(p + 4, std::forward<Args>(rest)...);
            }
        }
        result += *p++;
    }
    return result;
}

// =====================================================================
//  Macros — primary API
// =====================================================================
#define LOG_TRACE(module, fmt, ...)   Logger::Get().Log(LogLevel::TRACE,   module, (LogFormat(fmt, ##__VA_ARGS__)).c_str())
#define LOG_DEBUG(module, fmt, ...)   Logger::Get().Log(LogLevel::DEBUG,   module, (LogFormat(fmt, ##__VA_ARGS__)).c_str())
#define LOG_INFO(module, fmt, ...)    Logger::Get().Log(LogLevel::INFO,    module, (LogFormat(fmt, ##__VA_ARGS__)).c_str())
#define LOG_WARNING(module, fmt, ...) Logger::Get().Log(LogLevel::WARNING, module, (LogFormat(fmt, ##__VA_ARGS__)).c_str())
#define LOG_ERROR(module, fmt, ...)   Logger::Get().Log(LogLevel::ERR,     module, (LogFormat(fmt, ##__VA_ARGS__)).c_str())
#define LOG_FATAL(module, fmt, ...)   Logger::Get().Log(LogLevel::FATAL,   module, (LogFormat(fmt, ##__VA_ARGS__)).c_str())
