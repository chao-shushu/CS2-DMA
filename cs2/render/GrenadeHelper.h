#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>

#include "../game/Entity.h"
#include "../game/Game.h"
#include "../utils/Logger.h"
#include "../config/Language.h"

namespace fs = std::filesystem;

namespace GrenadeHelper
{
    // Grenade types
    enum GrenadeType {
        FLASH = 0,
        SMOKE = 1,
        HE = 2,
        MOLOTOV = 3,
        UNKNOWN = -1
    };

    // Grenade color mapping
    inline ImColor GetGrenadeColor(int type) {
        switch (type) {
            case FLASH:   return ImColor(255, 255, 0, 200);
            case SMOKE:   return ImColor(128, 128, 128, 200);
            case HE:      return ImColor(255, 128, 0, 200);
            case MOLOTOV: return ImColor(255, 64, 0, 200);
            default:      return ImColor(255, 255, 255, 200);
        }
    }

    // Throw position data
    struct ThrowPos {
        std::string Name;
        int Style;
        int Type;
        Vec3 Position;
        float Pitch;
        float Yaw;
    };

    // Map data
    struct MapData {
        std::string MapName;
        std::vector<ThrowPos> Throws;
    };

    // Settings
    inline bool Enabled = true;
    inline float MaxDistance = 2000.0f;
    inline float AimIndicatorDistance = 100.0f;
    inline bool ShowName = true;
    inline bool ShowLine = false;
    inline bool ShowBox = true;
    inline float BoxSize = 8.0f;

    // Recording settings
    inline int RecordHotKey = VK_F5;
    inline bool IsListeningForKey = false;
    inline char RecordHotKeyName[32] = "F5";
    inline bool AutoSave = true;
    inline int DefaultGrenadeType = 1;
    inline int DefaultThrowStyle = 0;
    inline int ThrowCounter = 0;
    inline bool NeedSave = false;

    // Data storage
    inline std::map<std::string, MapData> MapsData;
    inline std::string CurrentMap;
    inline std::vector<ThrowPos>* CurrentThrows = nullptr;

    // Pending throws
    struct PendingThrow {
        Vec3 Position;
        float Pitch;
        float Yaw;
        int Type;
        std::string Timestamp;
        bool Named;
        std::string Name;
        int Style;
    };
    inline std::vector<PendingThrow> PendingThrows;
    inline bool RecordKeyPressed = false;
    inline std::chrono::steady_clock::time_point LastRecordTime;

    // Function declarations
    const char* GetKeyName(int vkCode);
    void SetCustomHotKey(int vkCode);
    GrenadeType GetGrenadeTypeFromWeapon(const std::string& weaponName);
    std::string ExtractString(const std::string& content, size_t start, const std::string& key);
    int ExtractInt(const std::string& content, size_t start, const std::string& key);
    std::vector<float> ExtractArray(const std::string& content, size_t start, const std::string& key);
    size_t FindMatchingBracket(const std::string& content, size_t openPos);
    void LoadMapData(const std::string& helpPath = "GrenadeHelper");
    void UpdateMap(const std::string& mapName);
    void RecordPosition(const CEntity& LocalPlayer);
    void NamePendingThrow(int index, const std::string& name, int style, int type);
    void SaveToFile(const std::string& mapName);
    void DeletePendingThrow(int index);
    void ClearPendingThrows();
    float GetDistance(const Vec3& a, const Vec3& b);
    Vec3 AngleToDirection(float pitch, float yaw);
    bool DirectionToScreen(const Vec3& dir, Vec2& screenPos);
    void DrawDirectionArrow(ImDrawList* drawList, Vec2 screenCenter, Vec2 targetScreen, ImColor color, float distance);
    void DrawThrowPos(const ThrowPos& t, const CEntity& LocalPlayer, ImDrawList* drawList);
    const char* GetGrenadeTypeName(int type);
    const char* GetStyleName(int style);
    void Render(const CEntity& LocalPlayer);
    std::vector<std::string> GetAvailableMaps();
    void DeleteThrow(const std::string& mapName, int index);
    void UpdateThrow(const std::string& mapName, int index, const std::string& name, int style, int type,
                     float posX, float posY, float posZ, float pitch, float yaw);
    ThrowPos* GetThrow(const std::string& mapName, int index);
}
