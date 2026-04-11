#pragma once

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// =====================================================================
//  CrashHandler — SEH + std::terminate + MiniDump
//
//  On crash:
//    1. Writes detailed crash_YYYYMMDD_HHMMSS.log
//    2. Writes crash_YYYYMMDD_HHMMSS.dmp (MiniDump)
//    3. Dumps ring buffer (last N log entries)
//    4. Dumps feature state (ESP, Radar, GrenadeHelper, etc.)
//    5. Dumps system info (OS version, memory usage)
//    6. Shows MessageBox with crash file paths
// =====================================================================
namespace CrashHandler
{
    // Install all crash handlers. Call once at program start.
    void Install(const std::string& crashDir = "logs");

    // Internal — do not call directly
    LONG WINAPI SehFilter(EXCEPTION_POINTERS* pExInfo);
    void        TerminateHandler();
}
