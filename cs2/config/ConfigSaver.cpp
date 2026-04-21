#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "ConfigSaver.h"
#include "../game/MenuConfig.h"
#include "../render/GrenadeHelper.h"
#include "../utils/Logger.h"

namespace MyConfigSaver {

    void SaveConfig(const std::string& filename) {
        std::ofstream configFile(MenuConfig::path+'/'+filename);
        if (!configFile.is_open()) {
            LOG_DEBUG("Config", "SaveConfig: failed to open '{}'", MenuConfig::path + '/' + filename);
            return;
        }
        LOG_TRACE("Config", "SaveConfig: writing '{}'", filename);
       
        configFile << "ShowBoneESP " << MenuConfig::ShowBoneESP << std::endl;
        configFile << "ShowBoxESP " << MenuConfig::ShowBoxESP << std::endl;
        configFile << "ShowHealthBar " << MenuConfig::ShowHealthBar << std::endl;
        configFile << "ShowLineToEnemy " << MenuConfig::ShowLineToEnemy << std::endl;
        configFile << "ShowWeaponESP " << MenuConfig::ShowWeaponESP << std::endl;
        configFile << "ShowDistance " << MenuConfig::ShowDistance << std::endl;
        configFile << "ShowEyeRay " << MenuConfig::ShowEyeRay << std::endl;
        configFile << "ShowPlayerName " << MenuConfig::ShowPlayerName << std::endl;
        configFile << "HealthBarType " << MenuConfig::HealthBarType << std::endl;
        configFile << "BoneColor " << MenuConfig::BoneColor.Value.x << " " << MenuConfig::BoneColor.Value.y << " " << MenuConfig::BoneColor.Value.z << " " << MenuConfig::BoneColor.Value.w << std::endl;
        configFile << "LineToEnemyColor " << MenuConfig::LineToEnemyColor.Value.x << " " << MenuConfig::LineToEnemyColor.Value.y << " " << MenuConfig::LineToEnemyColor.Value.z << " " << MenuConfig::LineToEnemyColor.Value.w << std::endl;
        configFile << "BoxColor " << MenuConfig::BoxColor.Value.x << " " << MenuConfig::BoxColor.Value.y << " " << MenuConfig::BoxColor.Value.z << " " << MenuConfig::BoxColor.Value.w << std::endl;
        configFile << "EyeRayColor " << MenuConfig::EyeRayColor.Value.x << " " << MenuConfig::EyeRayColor.Value.y << " " << MenuConfig::EyeRayColor.Value.z << " " << MenuConfig::EyeRayColor.Value.w << std::endl;
        configFile << "ShowMenu " << MenuConfig::ShowMenu << std::endl;
        configFile << "BoxType " << MenuConfig::BoxType << std::endl;
        configFile << "TeamCheck " << MenuConfig::TeamCheck << std::endl;
        configFile << "Frames " << MenuConfig::MaxFrameRate << std::endl;
        configFile << "BoxThickness " << MenuConfig::BoxThickness << std::endl;
        configFile << "BoxRounding " << MenuConfig::BoxRounding << std::endl;
        configFile << "BoxFilled " << MenuConfig::BoxFilled << std::endl;
        configFile << "BoxFillAlpha " << MenuConfig::BoxFillAlpha << std::endl;
        configFile << "CornerLength " << MenuConfig::CornerLength << std::endl;
        configFile << "BoneThickness " << MenuConfig::BoneThickness << std::endl;
        configFile << "ShowHeadDot " << MenuConfig::ShowHeadDot << std::endl;
        configFile << "HeadDotColor " << MenuConfig::HeadDotColor.Value.x << " " << MenuConfig::HeadDotColor.Value.y << " " << MenuConfig::HeadDotColor.Value.z << " " << MenuConfig::HeadDotColor.Value.w << std::endl;
        configFile << "HeadDotSize " << MenuConfig::HeadDotSize << std::endl;
        configFile << "ShowArmorBar " << MenuConfig::ShowArmorBar << std::endl;
        configFile << "ArmorBarType " << MenuConfig::ArmorBarType << std::endl;
        configFile << "ArmorBarColor " << MenuConfig::ArmorBarColor.Value.x << " " << MenuConfig::ArmorBarColor.Value.y << " " << MenuConfig::ArmorBarColor.Value.z << " " << MenuConfig::ArmorBarColor.Value.w << std::endl;
        configFile << "ArmorBarWidth " << MenuConfig::ArmorBarWidth << std::endl;
        configFile << "HealthBarWidth " << MenuConfig::HealthBarWidth << std::endl;
        configFile << "EyeRayLength " << MenuConfig::EyeRayLength << std::endl;
        configFile << "EyeRayThickness " << MenuConfig::EyeRayThickness << std::endl;
        configFile << "LineToEnemyThickness " << MenuConfig::LineToEnemyThickness << std::endl;
        configFile << "LineToEnemyOrigin " << MenuConfig::LineToEnemyOrigin << std::endl;
        configFile << "NameColor " << MenuConfig::NameColor.Value.x << " " << MenuConfig::NameColor.Value.y << " " << MenuConfig::NameColor.Value.z << " " << MenuConfig::NameColor.Value.w << std::endl;
        configFile << "NameFontSize " << MenuConfig::NameFontSize << std::endl;
        configFile << "WeaponColor " << MenuConfig::WeaponColor.Value.x << " " << MenuConfig::WeaponColor.Value.y << " " << MenuConfig::WeaponColor.Value.z << " " << MenuConfig::WeaponColor.Value.w << std::endl;
        configFile << "WeaponFontSize " << MenuConfig::WeaponFontSize << std::endl;
        configFile << "DistanceColor " << MenuConfig::DistanceColor.Value.x << " " << MenuConfig::DistanceColor.Value.y << " " << MenuConfig::DistanceColor.Value.z << " " << MenuConfig::DistanceColor.Value.w << std::endl;
        configFile << "DistanceFontSize " << MenuConfig::DistanceFontSize << std::endl;
        configFile << "SafeZoneEnabled " << MenuConfig::SafeZoneEnabled << std::endl;
        configFile << "SafeZoneRadius " << MenuConfig::SafeZoneRadius << std::endl;
        configFile << "SafeZoneShape " << MenuConfig::SafeZoneShape << std::endl;
        configFile << "CrosshairEnabled " << MenuConfig::CrosshairEnabled << std::endl;
        configFile << "CrosshairSize " << MenuConfig::CrosshairSize << std::endl;
        configFile << "CrosshairThickness " << MenuConfig::CrosshairThickness << std::endl;
        configFile << "CrosshairGap " << MenuConfig::CrosshairGap << std::endl;
        configFile << "CrosshairStyle " << MenuConfig::CrosshairStyle << std::endl;
        configFile << "CrosshairColor " << MenuConfig::CrosshairColor.Value.x << " " << MenuConfig::CrosshairColor.Value.y << " " << MenuConfig::CrosshairColor.Value.z << " " << MenuConfig::CrosshairColor.Value.w << std::endl;
        configFile << "CrosshairOnEnemyColor " << MenuConfig::CrosshairOnEnemyColor << std::endl;
        configFile << "CrosshairEnemyColor " << MenuConfig::CrosshairEnemyColor.Value.x << " " << MenuConfig::CrosshairEnemyColor.Value.y << " " << MenuConfig::CrosshairEnemyColor.Value.z << " " << MenuConfig::CrosshairEnemyColor.Value.w << std::endl;
        configFile << "VSync " << MenuConfig::VSync << std::endl;
        configFile << "RenderWidth " << MenuConfig::RenderWidth << std::endl;
        configFile << "RenderHeight " << MenuConfig::RenderHeight << std::endl;
        configFile << "MonitorIndex " << MenuConfig::MonitorIndex << std::endl;
        configFile << "DebugLog " << MenuConfig::DebugLog << std::endl;
        configFile << "SelectedLanguage " << MenuConfig::SelectedLanguage << std::endl;
        configFile << "ShowWebRadar " << MenuConfig::ShowWebRadar << std::endl;
        configFile << "WebRadarPort " << MenuConfig::WebRadarPort << std::endl;
        configFile << "WebRadarInterval " << MenuConfig::WebRadarInterval << std::endl;
        configFile << "ShowBombESP " << MenuConfig::ShowBombESP << std::endl;
        configFile << "BombPlantedColor " << MenuConfig::BombPlantedColor.Value.x << " " << MenuConfig::BombPlantedColor.Value.y << " " << MenuConfig::BombPlantedColor.Value.z << " " << MenuConfig::BombPlantedColor.Value.w << std::endl;
        configFile << "BombCarrierColor " << MenuConfig::BombCarrierColor.Value.x << " " << MenuConfig::BombCarrierColor.Value.y << " " << MenuConfig::BombCarrierColor.Value.z << " " << MenuConfig::BombCarrierColor.Value.w << std::endl;
        configFile << "BombDroppedColor " << MenuConfig::BombDroppedColor.Value.x << " " << MenuConfig::BombDroppedColor.Value.y << " " << MenuConfig::BombDroppedColor.Value.z << " " << MenuConfig::BombDroppedColor.Value.w << std::endl;
        configFile << "BombDefusingColor " << MenuConfig::BombDefusingColor.Value.x << " " << MenuConfig::BombDefusingColor.Value.y << " " << MenuConfig::BombDefusingColor.Value.z << " " << MenuConfig::BombDefusingColor.Value.w << std::endl;
        configFile << "ShowSpectatorList " << MenuConfig::ShowSpectatorList << std::endl;
        configFile << "ShowPerfMonitor " << MenuConfig::ShowPerfMonitor << std::endl;
        configFile << "ShowProjectileESP " << MenuConfig::ShowProjectileESP << std::endl;
        configFile << "ShowProjectileRange " << MenuConfig::ShowProjectileRange << std::endl;
        configFile << "ProjectileRangeAlpha " << MenuConfig::ProjectileRangeAlpha << std::endl;
        configFile << "MenuHotKey " << MenuConfig::MenuHotKey << std::endl;

        configFile.close();
    }

    void LoadConfig(const std::string& filename) {
        std::string tempkey;

        std::ifstream configFile(MenuConfig::path + '/' + filename);
        if (!configFile.is_open()) {
            LOG_DEBUG("Config", "LoadConfig: file not found '{}'", MenuConfig::path + '/' + filename);
            return;
        }
        LOG_DEBUG("Config", "LoadConfig: reading '{}'", filename);

        std::string line;
        while (std::getline(configFile, line)) {
            std::istringstream iss(line);
            std::string key;
            if (iss >> key) {
                if (key == "ShowBoneESP") iss >> MenuConfig::ShowBoneESP;
                else if (key == "ShowBoxESP") iss >> MenuConfig::ShowBoxESP;
                else if (key == "ShowHealthBar") iss >> MenuConfig::ShowHealthBar;
                else if (key == "ShowLineToEnemy") iss >> MenuConfig::ShowLineToEnemy;
                else if (key == "ShowWeaponESP") iss >> MenuConfig::ShowWeaponESP;
                else if (key == "ShowDistance") iss >> MenuConfig::ShowDistance;
                else if (key == "ShowEyeRay") iss >> MenuConfig::ShowEyeRay;
                else if (key == "ShowPlayerName") iss >> MenuConfig::ShowPlayerName;
                else if (key == "HealthBarType") iss >> MenuConfig::HealthBarType;
                else if (key == "BoneColor") iss >> MenuConfig::BoneColor.Value.x >> MenuConfig::BoneColor.Value.y >> MenuConfig::BoneColor.Value.z >> MenuConfig::BoneColor.Value.w;
                else if (key == "LineToEnemyColor") iss >> MenuConfig::LineToEnemyColor.Value.x >> MenuConfig::LineToEnemyColor.Value.y >> MenuConfig::LineToEnemyColor.Value.z >> MenuConfig::LineToEnemyColor.Value.w;
                else if (key == "BoxColor") iss >> MenuConfig::BoxColor.Value.x >> MenuConfig::BoxColor.Value.y >> MenuConfig::BoxColor.Value.z >> MenuConfig::BoxColor.Value.w;
                else if (key == "EyeRayColor") iss >> MenuConfig::EyeRayColor.Value.x >> MenuConfig::EyeRayColor.Value.y >> MenuConfig::EyeRayColor.Value.z >> MenuConfig::EyeRayColor.Value.w;
                else if (key == "ShowMenu") iss >> MenuConfig::ShowMenu;
                else if (key == "BoxType") iss >> MenuConfig::BoxType;
                else if (key == "TeamCheck") iss >> MenuConfig::TeamCheck;
                else if (key == "Frames") iss >> MenuConfig::MaxFrameRate;
                else if (key == "BoxThickness") iss >> MenuConfig::BoxThickness;
                else if (key == "BoxRounding") iss >> MenuConfig::BoxRounding;
                else if (key == "BoxFilled") iss >> MenuConfig::BoxFilled;
                else if (key == "BoxFillAlpha") iss >> MenuConfig::BoxFillAlpha;
                else if (key == "CornerLength") iss >> MenuConfig::CornerLength;
                else if (key == "BoneThickness") iss >> MenuConfig::BoneThickness;
                else if (key == "ShowHeadDot") iss >> MenuConfig::ShowHeadDot;
                else if (key == "HeadDotColor") iss >> MenuConfig::HeadDotColor.Value.x >> MenuConfig::HeadDotColor.Value.y >> MenuConfig::HeadDotColor.Value.z >> MenuConfig::HeadDotColor.Value.w;
                else if (key == "HeadDotSize") iss >> MenuConfig::HeadDotSize;
                else if (key == "ShowArmorBar") iss >> MenuConfig::ShowArmorBar;
                else if (key == "ArmorBarType") iss >> MenuConfig::ArmorBarType;
                else if (key == "ArmorBarColor") iss >> MenuConfig::ArmorBarColor.Value.x >> MenuConfig::ArmorBarColor.Value.y >> MenuConfig::ArmorBarColor.Value.z >> MenuConfig::ArmorBarColor.Value.w;
                else if (key == "ArmorBarWidth") iss >> MenuConfig::ArmorBarWidth;
                else if (key == "HealthBarWidth") iss >> MenuConfig::HealthBarWidth;
                else if (key == "EyeRayLength") iss >> MenuConfig::EyeRayLength;
                else if (key == "EyeRayThickness") iss >> MenuConfig::EyeRayThickness;
                else if (key == "LineToEnemyThickness") iss >> MenuConfig::LineToEnemyThickness;
                else if (key == "LineToEnemyOrigin") iss >> MenuConfig::LineToEnemyOrigin;
                else if (key == "NameColor") iss >> MenuConfig::NameColor.Value.x >> MenuConfig::NameColor.Value.y >> MenuConfig::NameColor.Value.z >> MenuConfig::NameColor.Value.w;
                else if (key == "NameFontSize") iss >> MenuConfig::NameFontSize;
                else if (key == "WeaponColor") iss >> MenuConfig::WeaponColor.Value.x >> MenuConfig::WeaponColor.Value.y >> MenuConfig::WeaponColor.Value.z >> MenuConfig::WeaponColor.Value.w;
                else if (key == "WeaponFontSize") iss >> MenuConfig::WeaponFontSize;
                else if (key == "DistanceColor") iss >> MenuConfig::DistanceColor.Value.x >> MenuConfig::DistanceColor.Value.y >> MenuConfig::DistanceColor.Value.z >> MenuConfig::DistanceColor.Value.w;
                else if (key == "DistanceFontSize") iss >> MenuConfig::DistanceFontSize;
                else if (key == "SafeZoneEnabled") iss >> MenuConfig::SafeZoneEnabled;
                else if (key == "SafeZoneRadius") iss >> MenuConfig::SafeZoneRadius;
                else if (key == "SafeZoneShape") iss >> MenuConfig::SafeZoneShape;
                else if (key == "CrosshairEnabled") iss >> MenuConfig::CrosshairEnabled;
                else if (key == "CrosshairSize") iss >> MenuConfig::CrosshairSize;
                else if (key == "CrosshairThickness") iss >> MenuConfig::CrosshairThickness;
                else if (key == "CrosshairGap") iss >> MenuConfig::CrosshairGap;
                else if (key == "CrosshairStyle") iss >> MenuConfig::CrosshairStyle;
                else if (key == "CrosshairColor") iss >> MenuConfig::CrosshairColor.Value.x >> MenuConfig::CrosshairColor.Value.y >> MenuConfig::CrosshairColor.Value.z >> MenuConfig::CrosshairColor.Value.w;
                else if (key == "CrosshairOnEnemyColor") iss >> MenuConfig::CrosshairOnEnemyColor;
                else if (key == "CrosshairEnemyColor") iss >> MenuConfig::CrosshairEnemyColor.Value.x >> MenuConfig::CrosshairEnemyColor.Value.y >> MenuConfig::CrosshairEnemyColor.Value.z >> MenuConfig::CrosshairEnemyColor.Value.w;
                else if (key == "VSync") iss >> MenuConfig::VSync;
                else if (key == "RenderWidth") iss >> MenuConfig::RenderWidth;
                else if (key == "RenderHeight") iss >> MenuConfig::RenderHeight;
                else if (key == "MonitorIndex") iss >> MenuConfig::MonitorIndex;
                else if (key == "DebugLog") iss >> MenuConfig::DebugLog;
                else if (key == "SelectedLanguage") iss >> MenuConfig::SelectedLanguage;
                else if (key == "ShowWebRadar") iss >> MenuConfig::ShowWebRadar;
                else if (key == "WebRadarPort") iss >> MenuConfig::WebRadarPort;
                else if (key == "WebRadarInterval") iss >> MenuConfig::WebRadarInterval;
                else if (key == "ShowBombESP") iss >> MenuConfig::ShowBombESP;
                else if (key == "BombPlantedColor") iss >> MenuConfig::BombPlantedColor.Value.x >> MenuConfig::BombPlantedColor.Value.y >> MenuConfig::BombPlantedColor.Value.z >> MenuConfig::BombPlantedColor.Value.w;
                else if (key == "BombCarrierColor") iss >> MenuConfig::BombCarrierColor.Value.x >> MenuConfig::BombCarrierColor.Value.y >> MenuConfig::BombCarrierColor.Value.z >> MenuConfig::BombCarrierColor.Value.w;
                else if (key == "BombDroppedColor") iss >> MenuConfig::BombDroppedColor.Value.x >> MenuConfig::BombDroppedColor.Value.y >> MenuConfig::BombDroppedColor.Value.z >> MenuConfig::BombDroppedColor.Value.w;
                else if (key == "BombDefusingColor") iss >> MenuConfig::BombDefusingColor.Value.x >> MenuConfig::BombDefusingColor.Value.y >> MenuConfig::BombDefusingColor.Value.z >> MenuConfig::BombDefusingColor.Value.w;
                else if (key == "ShowSpectatorList") iss >> MenuConfig::ShowSpectatorList;
                else if (key == "ShowPerfMonitor") iss >> MenuConfig::ShowPerfMonitor;
                else if (key == "ShowProjectileESP") iss >> MenuConfig::ShowProjectileESP;
                else if (key == "ShowProjectileRange") iss >> MenuConfig::ShowProjectileRange;
                else if (key == "ProjectileRangeAlpha") iss >> MenuConfig::ProjectileRangeAlpha;
                else if (key == "MenuHotKey") {
                    iss >> MenuConfig::MenuHotKey;
                    strcpy_s(MenuConfig::MenuHotKeyName, GrenadeHelper::GetKeyName(MenuConfig::MenuHotKey));
                }
            }
        }

        configFile.close();
        LOG_DEBUG("Config", "LoadConfig: done, DebugLog={} ShowWebRadar={} Language={}", MenuConfig::DebugLog, MenuConfig::ShowWebRadar, MenuConfig::SelectedLanguage);
    }

    static bool g_configDirty = false;

    void MarkDirty()  { g_configDirty = true; }
    bool IsDirty()    { return g_configDirty; }
    void ClearDirty() { g_configDirty = false; }
}
