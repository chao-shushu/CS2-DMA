#pragma once

// =====================================================================
//  Telemetry — beta test log uploader
//
//  When BETA_TELEMETRY is defined at compile time, this module:
//    - Tracks the current log file path
//    - On session end (normal exit or crash), uploads the log file
//      to a GitHub repository via the Contents API
//    - Crash reports (.log + .dmp) are also uploaded
//
//  When BETA_TELEMETRY is NOT defined, all functions compile to no-ops.
// =====================================================================

#include <string>

namespace Telemetry
{
    // Call once at startup (after Logger::Init)
    void Init();

    // Set the current session log file path (called by Logger)
    void SetLogFilePath(const std::string& path);

    // Set the current crash file paths (called by CrashHandler)
    void SetCrashFiles(const std::string& logPath, const std::string& dmpPath);

    // Upload session log file (called on normal exit)
    void UploadSessionLog();

    // Upload crash files (called from crash handler)
    void UploadCrashFiles();
}
