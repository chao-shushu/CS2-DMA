#include "CrashHandler.h"
#include "Logger.h"
#include "Telemetry.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstdio>
#include <filesystem>

#include <windows.h>
#include <dbghelp.h>
#include <psapi.h>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")

// Forward declarations for feature state
namespace MenuConfig
{
    extern bool ShowBoneESP;
    extern bool ShowBoxESP;
    extern bool ShowHealthBar;
    extern bool ShowWeaponESP;
    extern bool ShowDistance;
    extern bool ShowEyeRay;
    extern bool ShowPlayerName;
    extern bool ShowLineToEnemy;
    extern bool TeamCheck;
    extern int  BoxType;
}

namespace GrenadeHelper
{
    extern bool        Enabled;
    extern std::string CurrentMap;
}

namespace fs = std::filesystem;

// =====================================================================
//  Internal state
// =====================================================================
static std::string g_CrashDir = "logs";

// =====================================================================
//  Helpers
// =====================================================================
static std::string CrashTimestamp()
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

static std::string CrashFilePrefix()
{
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    struct tm ti{};
    localtime_s(&ti, &t);

    char buf[64];
    snprintf(buf, sizeof(buf), "%s/crash_%04d%02d%02d_%02d%02d%02d",
             g_CrashDir.c_str(),
             ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
             ti.tm_hour, ti.tm_min, ti.tm_sec);
    return buf;
}

static const char* ExceptionCodeName(DWORD code)
{
    switch (code)
    {
    case EXCEPTION_ACCESS_VIOLATION:         return "ACCESS_VIOLATION";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "ARRAY_BOUNDS_EXCEEDED";
    case EXCEPTION_BREAKPOINT:               return "BREAKPOINT";
    case EXCEPTION_DATATYPE_MISALIGNMENT:    return "DATATYPE_MISALIGNMENT";
    case EXCEPTION_FLT_DENORMAL_OPERAND:     return "FLT_DENORMAL_OPERAND";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "FLT_DIVIDE_BY_ZERO";
    case EXCEPTION_FLT_INEXACT_RESULT:       return "FLT_INEXACT_RESULT";
    case EXCEPTION_FLT_INVALID_OPERATION:    return "FLT_INVALID_OPERATION";
    case EXCEPTION_FLT_OVERFLOW:             return "FLT_OVERFLOW";
    case EXCEPTION_FLT_STACK_CHECK:          return "FLT_STACK_CHECK";
    case EXCEPTION_FLT_UNDERFLOW:            return "FLT_UNDERFLOW";
    case EXCEPTION_ILLEGAL_INSTRUCTION:      return "ILLEGAL_INSTRUCTION";
    case EXCEPTION_IN_PAGE_ERROR:            return "IN_PAGE_ERROR";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "INT_DIVIDE_BY_ZERO";
    case EXCEPTION_INT_OVERFLOW:             return "INT_OVERFLOW";
    case EXCEPTION_INVALID_DISPOSITION:      return "INVALID_DISPOSITION";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "NONCONTINUABLE_EXCEPTION";
    case EXCEPTION_PRIV_INSTRUCTION:         return "PRIV_INSTRUCTION";
    case EXCEPTION_SINGLE_STEP:              return "SINGLE_STEP";
    case EXCEPTION_STACK_OVERFLOW:           return "STACK_OVERFLOW";
    default:                                 return "UNKNOWN";
    }
}

// =====================================================================
//  Stack walk using StackWalk64 + SymFromAddr
// =====================================================================
static std::string WalkStack(CONTEXT* ctx)
{
    std::stringstream ss;

    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread  = GetCurrentThread();

    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    SymInitialize(hProcess, NULL, TRUE);

    STACKFRAME64 frame{};
    DWORD machineType = 0;

#ifdef _M_X64
    machineType           = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Offset   = ctx->Rip;
    frame.AddrPC.Mode     = AddrModeFlat;
    frame.AddrFrame.Offset = ctx->Rbp;
    frame.AddrFrame.Mode  = AddrModeFlat;
    frame.AddrStack.Offset = ctx->Rsp;
    frame.AddrStack.Mode  = AddrModeFlat;
#else
    machineType           = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset   = ctx->Eip;
    frame.AddrPC.Mode     = AddrModeFlat;
    frame.AddrFrame.Offset = ctx->Ebp;
    frame.AddrFrame.Mode  = AddrModeFlat;
    frame.AddrStack.Offset = ctx->Esp;
    frame.AddrStack.Mode  = AddrModeFlat;
#endif

    ss << "  Call Stack:\n";
    ss << "  --------------------------------------------------\n";

    for (int i = 0; i < 64; i++)
    {
        if (!StackWalk64(machineType, hProcess, hThread, &frame,
                         ctx, NULL, SymFunctionTableAccess64,
                         SymGetModuleBase64, NULL))
            break;

        if (frame.AddrPC.Offset == 0)
            break;

        DWORD64 addr = frame.AddrPC.Offset;

        // Module name
        HMODULE hMod = NULL;
        char modName[MAX_PATH] = "???";
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                               GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               (LPCSTR)addr, &hMod))
        {
            GetModuleFileNameA(hMod, modName, MAX_PATH);
            // Extract just filename
            char* slash = strrchr(modName, '\\');
            if (slash) memmove(modName, slash + 1, strlen(slash));
        }

        // Symbol name
        char symBuf[sizeof(SYMBOL_INFO) + 256]{};
        SYMBOL_INFO* sym = (SYMBOL_INFO*)symBuf;
        sym->SizeOfStruct = sizeof(SYMBOL_INFO);
        sym->MaxNameLen   = 255;

        DWORD64 displacement64 = 0;
        const char* funcName = "???";
        if (SymFromAddr(hProcess, addr, &displacement64, sym))
            funcName = sym->Name;

        // Source line
        IMAGEHLP_LINE64 lineInfo{};
        lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD displacement32 = 0;
        bool hasLine = SymGetLineFromAddr64(hProcess, addr, &displacement32, &lineInfo);

        char frameBuf[512];
        if (hasLine)
        {
            snprintf(frameBuf, sizeof(frameBuf),
                     "  [%2d] 0x%016llX  %s!%s + 0x%llX  (%s:%lu)",
                     i, (unsigned long long)addr, modName, funcName,
                     (unsigned long long)displacement64,
                     lineInfo.FileName, lineInfo.LineNumber);
        }
        else
        {
            snprintf(frameBuf, sizeof(frameBuf),
                     "  [%2d] 0x%016llX  %s!%s + 0x%llX",
                     i, (unsigned long long)addr, modName, funcName,
                     (unsigned long long)displacement64);
        }
        ss << frameBuf << "\n";
    }

    SymCleanup(hProcess);
    return ss.str();
}

// =====================================================================
//  System info
// =====================================================================
static std::string GetSystemInfo()
{
    std::stringstream ss;

    // OS version
    OSVERSIONINFOEXA osvi{};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    // Use RtlGetVersion to avoid manifest requirement
    typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (hNtdll)
    {
        auto fn = (RtlGetVersionPtr)GetProcAddress(hNtdll, "RtlGetVersion");
        if (fn)
        {
            RTL_OSVERSIONINFOW rovi{};
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if (fn(&rovi) == 0)
            {
                ss << "  OS Version: Windows " << rovi.dwMajorVersion << "."
                   << rovi.dwMinorVersion << " Build " << rovi.dwBuildNumber << "\n";
            }
        }
    }

    // Memory
    MEMORYSTATUSEX memStat{};
    memStat.dwLength = sizeof(memStat);
    if (GlobalMemoryStatusEx(&memStat))
    {
        ss << "  Physical Memory: "
           << (memStat.ullTotalPhys / (1024 * 1024)) << " MB total, "
           << (memStat.ullAvailPhys / (1024 * 1024)) << " MB available\n";
        ss << "  Memory Load: " << memStat.dwMemoryLoad << "%\n";
    }

    // Process memory
    PROCESS_MEMORY_COUNTERS_EX pmc{};
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
    {
        ss << "  Process Working Set: " << (pmc.WorkingSetSize / (1024 * 1024)) << " MB\n";
        ss << "  Process Private Bytes: " << (pmc.PrivateUsage / (1024 * 1024)) << " MB\n";
    }

    return ss.str();
}

// =====================================================================
//  Feature state dump
// =====================================================================
static std::string GetFeatureState()
{
    std::stringstream ss;
    ss << "  ESP:\n";
    ss << "    BoxESP:      " << (MenuConfig::ShowBoxESP ? "ON" : "OFF")
       << " (Type: " << MenuConfig::BoxType << ")\n";
    ss << "    BoneESP:     " << (MenuConfig::ShowBoneESP ? "ON" : "OFF") << "\n";
    ss << "    HealthBar:   " << (MenuConfig::ShowHealthBar ? "ON" : "OFF") << "\n";
    ss << "    WeaponESP:   " << (MenuConfig::ShowWeaponESP ? "ON" : "OFF") << "\n";
    ss << "    DistanceESP: " << (MenuConfig::ShowDistance ? "ON" : "OFF") << "\n";
    ss << "    PlayerName:  " << (MenuConfig::ShowPlayerName ? "ON" : "OFF") << "\n";
    ss << "    EyeRay:      " << (MenuConfig::ShowEyeRay ? "ON" : "OFF") << "\n";
    ss << "    LineToEnemy: " << (MenuConfig::ShowLineToEnemy ? "ON" : "OFF") << "\n";
    ss << "    TeamCheck:   " << (MenuConfig::TeamCheck ? "ON" : "OFF") << "\n";
    ss << "  GrenadeHelper: " << (GrenadeHelper::Enabled ? "ON" : "OFF")
       << " (Map: " << (GrenadeHelper::CurrentMap.empty() ? "none" : GrenadeHelper::CurrentMap) << ")\n";
    return ss.str();
}

// =====================================================================
//  MiniDump
// =====================================================================
static bool WriteMiniDump(const std::string& path, EXCEPTION_POINTERS* pExInfo)
{
    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    MINIDUMP_EXCEPTION_INFORMATION mei{};
    mei.ThreadId          = GetCurrentThreadId();
    mei.ExceptionPointers = pExInfo;
    mei.ClientPointers    = FALSE;

    BOOL ok = MiniDumpWriteDump(
        GetCurrentProcess(), GetCurrentProcessId(), hFile,
        (MINIDUMP_TYPE)(MiniDumpWithDataSegs | MiniDumpWithHandleData |
                        MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules),
        pExInfo ? &mei : NULL, NULL, NULL);

    CloseHandle(hFile);
    return ok != FALSE;
}

// =====================================================================
//  Full crash report
// =====================================================================
static void WriteCrashReport(const std::string& logPath, EXCEPTION_POINTERS* pExInfo)
{
    std::ofstream f(logPath, std::ios::out | std::ios::trunc);
    if (!f.is_open())
        return;

    f << "================================================================\n";
    f << "  CS2-DMA CRASH REPORT\n";
    f << "  Time: " << CrashTimestamp() << "\n";
    f << "================================================================\n\n";

    // Exception info
    if (pExInfo && pExInfo->ExceptionRecord)
    {
        DWORD code    = pExInfo->ExceptionRecord->ExceptionCode;
        void* addr    = pExInfo->ExceptionRecord->ExceptionAddress;

        f << "[Exception]\n";
        f << "  Code:    0x" << std::hex << code << std::dec
          << " (" << ExceptionCodeName(code) << ")\n";
        f << "  Address: 0x" << std::hex << (DWORD64)addr << std::dec << "\n";

        if (code == EXCEPTION_ACCESS_VIOLATION && pExInfo->ExceptionRecord->NumberParameters >= 2)
        {
            ULONG_PTR* info = pExInfo->ExceptionRecord->ExceptionInformation;
            f << "  Access:  " << (info[0] == 0 ? "READ" : (info[0] == 1 ? "WRITE" : "DEP"))
              << " at 0x" << std::hex << info[1] << std::dec << "\n";
        }
        f << "\n";

        // Registers
        CONTEXT* ctx = pExInfo->ContextRecord;
        if (ctx)
        {
            f << "[Registers]\n";
#ifdef _M_X64
            char regBuf[512];
            snprintf(regBuf, sizeof(regBuf),
                     "  RAX=0x%016llX  RBX=0x%016llX  RCX=0x%016llX  RDX=0x%016llX\n"
                     "  RSI=0x%016llX  RDI=0x%016llX  RBP=0x%016llX  RSP=0x%016llX\n"
                     "  R8 =0x%016llX  R9 =0x%016llX  R10=0x%016llX  R11=0x%016llX\n"
                     "  R12=0x%016llX  R13=0x%016llX  R14=0x%016llX  R15=0x%016llX\n"
                     "  RIP=0x%016llX  EFLAGS=0x%08lX\n",
                     ctx->Rax, ctx->Rbx, ctx->Rcx, ctx->Rdx,
                     ctx->Rsi, ctx->Rdi, ctx->Rbp, ctx->Rsp,
                     ctx->R8, ctx->R9, ctx->R10, ctx->R11,
                     ctx->R12, ctx->R13, ctx->R14, ctx->R15,
                     ctx->Rip, ctx->EFlags);
            f << regBuf;
#else
            char regBuf[256];
            snprintf(regBuf, sizeof(regBuf),
                     "  EAX=0x%08lX  EBX=0x%08lX  ECX=0x%08lX  EDX=0x%08lX\n"
                     "  ESI=0x%08lX  EDI=0x%08lX  EBP=0x%08lX  ESP=0x%08lX\n"
                     "  EIP=0x%08lX  EFLAGS=0x%08lX\n",
                     ctx->Eax, ctx->Ebx, ctx->Ecx, ctx->Edx,
                     ctx->Esi, ctx->Edi, ctx->Ebp, ctx->Esp,
                     ctx->Eip, ctx->EFlags);
            f << regBuf;
#endif
            f << "\n";

            // Stack trace
            f << "[Stack Trace]\n";
            f << WalkStack(ctx);
            f << "\n";
        }
    }
    else
    {
        f << "[Exception]\n";
        f << "  std::terminate() or unknown fatal error (no EXCEPTION_POINTERS)\n\n";

        // Capture current stack
        f << "[Stack Trace (captured)]\n";
        void* stack[64];
        USHORT frames = CaptureStackBackTrace(2, 64, stack, NULL);
        for (USHORT i = 0; i < frames; i++)
        {
            char buf[128];
            snprintf(buf, sizeof(buf), "  [%2d] 0x%016llX", i, (unsigned long long)stack[i]);
            f << buf << "\n";
        }
        f << "\n";
    }

    // Loaded modules
    f << "[Loaded Modules]\n";
    {
        HANDLE hProcess = GetCurrentProcess();
        HMODULE hMods[256];
        DWORD cbNeeded = 0;
        if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
        {
            int count = cbNeeded / sizeof(HMODULE);
            for (int i = 0; i < count && i < 256; i++)
            {
                char modName[MAX_PATH];
                if (GetModuleFileNameA(hMods[i], modName, MAX_PATH))
                {
                    MODULEINFO mi{};
                    GetModuleInformation(hProcess, hMods[i], &mi, sizeof(mi));
                    char line[512];
                    snprintf(line, sizeof(line), "  0x%016llX  Size: %6lu KB  %s",
                             (unsigned long long)mi.lpBaseOfDll,
                             (unsigned long)mi.SizeOfImage / 1024, modName);
                    f << line << "\n";
                }
            }
        }
    }
    f << "\n";

    // Feature state
    f << "[Feature State]\n";
    f << GetFeatureState();
    f << "\n";

    // System info
    f << "[System Info]\n";
    f << GetSystemInfo();
    f << "\n";

    // Recent logs from ring buffer
    f << "[Recent Log Entries (Ring Buffer)]\n";
    f << "  --------------------------------------------------\n";
    {
        auto snap = Logger::Get().GetRecentLogs();
        for (int i = 0; i < snap.Count; i++)
        {
            const auto& e = snap.Entries[i];
            const char* lvl = "???";
            switch (e.Level)
            {
            case LogLevel::TRACE:   lvl = "TRACE";   break;
            case LogLevel::DEBUG:   lvl = "DEBUG";   break;
            case LogLevel::INFO:    lvl = "INFO";    break;
            case LogLevel::WARNING: lvl = "WARNING"; break;
            case LogLevel::ERR:     lvl = "ERROR";   break;
            case LogLevel::FATAL:   lvl = "FATAL";   break;
            }
            char line[640];
            snprintf(line, sizeof(line), "  [%s] [%s] [%s] %s",
                     e.Timestamp, lvl, e.Module, e.Message);
            f << line << "\n";
        }
        if (snap.Count == 0)
            f << "  (no log entries captured)\n";
    }
    f << "\n";

    // Session log file reference
    f << "[Session Log File]\n";
    f << "  " << Logger::Get().GetLogFilePath() << "\n";

    f << "\n================================================================\n";
    f << "  END OF CRASH REPORT\n";
    f << "================================================================\n";

    f.close();
}

// =====================================================================
//  SEH handler
// =====================================================================
LONG WINAPI CrashHandler::SehFilter(EXCEPTION_POINTERS* pExInfo)
{
    // Avoid re-entrance
    static volatile LONG inHandler = 0;
    if (InterlockedCompareExchange(&inHandler, 1, 0) != 0)
        return EXCEPTION_EXECUTE_HANDLER;

    std::string prefix = CrashFilePrefix();
    std::string logPath = prefix + ".log";
    std::string dmpPath = prefix + ".dmp";

    // Ensure crash dir exists
    if (!fs::exists(g_CrashDir))
        fs::create_directories(g_CrashDir);

    // Write MiniDump first (most critical)
    bool dmpOk = WriteMiniDump(dmpPath, pExInfo);

    // Write crash report
    WriteCrashReport(logPath, pExInfo);

    // Console output
    DWORD code = pExInfo ? pExInfo->ExceptionRecord->ExceptionCode : 0;
    fprintf(stderr,
            "\n\033[38;5;196m"
            "!!! CRASH: %s (0x%08lX) !!!\n"
            "  Crash log: %s\n"
            "  MiniDump:  %s (%s)\n"
            "\033[0m\n",
            ExceptionCodeName(code), code,
            logPath.c_str(),
            dmpPath.c_str(), dmpOk ? "OK" : "FAILED");

    // Upload crash files to telemetry (before MessageBox blocks)
    Telemetry::SetCrashFiles(logPath, dmpOk ? dmpPath : "");
    Telemetry::UploadCrashFiles();

    // MessageBox
    char msg[512];
    snprintf(msg, sizeof(msg),
             "CS2-DMA has crashed!\n\n"
             "Exception: %s (0x%08lX)\n\n"
             "Crash log: %s\n"
             "MiniDump: %s\n\n"
             "Please report this crash with the above files.",
             ExceptionCodeName(code), code,
             logPath.c_str(), dmpPath.c_str());
    MessageBoxA(NULL, msg, "CS2-DMA Crash", MB_OK | MB_ICONERROR);

    return EXCEPTION_EXECUTE_HANDLER;
}

// =====================================================================
//  std::terminate handler
// =====================================================================
void CrashHandler::TerminateHandler()
{
    std::string prefix = CrashFilePrefix();
    std::string logPath = prefix + "_terminate.log";

    if (!fs::exists(g_CrashDir))
        fs::create_directories(g_CrashDir);

    WriteCrashReport(logPath, nullptr);

    // Upload crash files to telemetry
    Telemetry::SetCrashFiles(logPath, "");
    Telemetry::UploadCrashFiles();

    fprintf(stderr,
            "\n\033[38;5;196m"
            "!!! std::terminate() called !!!\n"
            "  Crash log: %s\n"
            "\033[0m\n",
            logPath.c_str());

    char msg[256];
    snprintf(msg, sizeof(msg),
             "CS2-DMA: std::terminate() called!\n\n"
             "Crash log: %s\n\n"
             "Please report this crash.",
             logPath.c_str());
    MessageBoxA(NULL, msg, "CS2-DMA Fatal Error", MB_OK | MB_ICONERROR);

    std::abort();
}

// =====================================================================
//  Install
// =====================================================================
void CrashHandler::Install(const std::string& crashDir)
{
    g_CrashDir = crashDir;

    if (!fs::exists(g_CrashDir))
        fs::create_directories(g_CrashDir);

    SetUnhandledExceptionFilter(SehFilter);
    std::set_terminate(TerminateHandler);

    LOG_INFO("CrashHandler", "Crash handlers installed (output: {})", crashDir);
}
