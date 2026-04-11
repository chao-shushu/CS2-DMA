#include "SettingsManager.h"
#include "../utils/Logger.h"

#include "rapidjson/document.h"

#include <fstream>
#include <sstream>
#include <filesystem>

void SettingsManager::LoadSettings()
{
    if (!std::filesystem::exists("config.json")) {
        LOG_WARNING("Config", "Config file not found!");
        return;
    }
	LOG_INFO("Config", "Config file found");
    std::string readeddata;
    std::ifstream file("config.json");
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    readeddata = buffer.str();
    rapidjson::Document configDoc;
    configDoc.Parse(readeddata.c_str());

    if (configDoc.HasMember("en"))
        this->language = configDoc["en"].GetString();
}
