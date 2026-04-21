#pragma once

#include <string>

namespace MyConfigSaver {
    extern void SaveConfig(const std::string& filename);

    extern void LoadConfig(const std::string& filename);

    // Dirty flag: mark config as changed, save immediately on next auto-save tick
    extern void MarkDirty();
    extern bool IsDirty();
    extern void ClearDirty();
}
