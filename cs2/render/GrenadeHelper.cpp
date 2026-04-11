#include "GrenadeHelper.h"

namespace GrenadeHelper
{
    const char* GetKeyName(int vkCode) {
        static char name[32];

        // Mouse buttons
        if (vkCode == VK_LBUTTON) return "Mouse Left";
        if (vkCode == VK_RBUTTON) return "Mouse Right";
        if (vkCode == VK_MBUTTON) return "Mouse Middle";
        if (vkCode == VK_XBUTTON1) return "Mouse Side1";
        if (vkCode == VK_XBUTTON2) return "Mouse Side2";

        // Function keys
        if (vkCode >= VK_F1 && vkCode <= VK_F24) {
            sprintf_s(name, "F%d", vkCode - VK_F1 + 1);
            return name;
        }

        // Special keys
        if (vkCode == VK_INSERT) return "Insert";
        if (vkCode == VK_DELETE) return "Delete";
        if (vkCode == VK_HOME) return "Home";
        if (vkCode == VK_END) return "End";
        if (vkCode == VK_PRIOR) return "PageUp";
        if (vkCode == VK_NEXT) return "PageDown";
        if (vkCode == VK_SPACE) return "Space";
        if (vkCode == VK_TAB) return "Tab";
        if (vkCode == VK_ESCAPE) return "Escape";
        if (vkCode == VK_RETURN) return "Enter";
        if (vkCode == VK_BACK) return "Backspace";

        // Number keys
        if (vkCode >= 0x30 && vkCode <= 0x39) {
            sprintf_s(name, "%c", '0' + (vkCode - 0x30));
            return name;
        }

        // Letter keys
        if (vkCode >= 0x41 && vkCode <= 0x5A) {
            sprintf_s(name, "%c", 'A' + (vkCode - 0x41));
            return name;
        }

        // Numpad
        if (vkCode >= VK_NUMPAD0 && vkCode <= VK_NUMPAD9) {
            sprintf_s(name, "Num%d", vkCode - VK_NUMPAD0);
            return name;
        }

        sprintf_s(name, "Key%d", vkCode);
        return name;
    }

    void SetCustomHotKey(int vkCode) {
        RecordHotKey = vkCode;
        strcpy_s(RecordHotKeyName, GetKeyName(vkCode));
        IsListeningForKey = false;
    }

    GrenadeType GetGrenadeTypeFromWeapon(const std::string& weaponName)
    {
        std::string weapon = weaponName;
        for (auto& c : weapon) c = tolower(c);

        // Check molotov/incendiary FIRST (before grenade) - CT incendiary contains "grenade"
        // CT: weapon_incgrenade, T: weapon_molotov
        if (weapon.find("molotov") != std::string::npos || weapon.find("inc") != std::string::npos)
            return MOLOTOV;
        if (weapon.find("flash") != std::string::npos)
            return FLASH;
        if (weapon.find("smoke") != std::string::npos)
            return SMOKE;
        if (weapon.find("hegrenade") != std::string::npos || weapon.find("he") != std::string::npos)
            return HE;

        return UNKNOWN;
    }

    // Helper: extract string value from "Key":"Value"
    std::string ExtractString(const std::string& content, size_t start, const std::string& key)
    {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = content.find(searchKey, start);
        if (keyPos == std::string::npos) return "";
        
        size_t colonPos = content.find(":", keyPos);
        if (colonPos == std::string::npos) return "";
        
        size_t quoteStart = content.find("\"", colonPos);
        if (quoteStart == std::string::npos) return "";
        
        size_t quoteEnd = content.find("\"", quoteStart + 1);
        if (quoteEnd == std::string::npos) return "";
        
        return content.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
    }

    // Helper: extract int value from "Key":number
    int ExtractInt(const std::string& content, size_t start, const std::string& key)
    {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = content.find(searchKey, start);
        if (keyPos == std::string::npos) return 0;
        
        size_t colonPos = content.find(":", keyPos);
        if (colonPos == std::string::npos) return 0;
        
        // Skip whitespace
        size_t numStart = colonPos + 1;
        while (numStart < content.size() && (content[numStart] == ' ' || content[numStart] == '\n' || content[numStart] == '\t'))
            numStart++;
        
        // Find end of number
        size_t numEnd = numStart;
        while (numEnd < content.size() && content[numEnd] != ',' && content[numEnd] != '}' && content[numEnd] != '\n')
            numEnd++;
        
        std::string numStr = content.substr(numStart, numEnd - numStart);
        try {
            return std::stoi(numStr);
        }
        catch (...) {
            return 0;  // Return 0 on parse error
        }
    }

    // Helper: extract float array from "Key":[a,b,c]
    std::vector<float> ExtractArray(const std::string& content, size_t start, const std::string& key)
    {
        std::vector<float> result;
        
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = content.find(searchKey, start);
        if (keyPos == std::string::npos) return result;
        
        size_t bracketStart = content.find("[", keyPos);
        if (bracketStart == std::string::npos) return result;
        
        size_t bracketEnd = content.find("]", bracketStart);
        if (bracketEnd == std::string::npos) return result;
        
        std::string arrayContent = content.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
        
        // Parse comma-separated numbers
        size_t pos = 0;
        while (pos < arrayContent.size())
        {
            // Skip whitespace
            while (pos < arrayContent.size() && (arrayContent[pos] == ' ' || arrayContent[pos] == '\n' || arrayContent[pos] == '\t'))
                pos++;
            
            if (pos >= arrayContent.size()) break;
            
            // Find number
            size_t numStart = pos;
            while (pos < arrayContent.size() && arrayContent[pos] != ',' && arrayContent[pos] != ' ' && arrayContent[pos] != '\n')
                pos++;
            
            std::string numStr = arrayContent.substr(numStart, pos - numStart);
            if (!numStr.empty()) {
                try { result.push_back(std::stof(numStr)); }
                catch (...) {}
            }
            
            // Skip comma
            while (pos < arrayContent.size() && (arrayContent[pos] == ',' || arrayContent[pos] == ' '))
                pos++;
        }
        
        return result;
    }

    // Find matching closing bracket for array
    size_t FindMatchingBracket(const std::string& content, size_t openPos)
    {
        int depth = 0;
        for (size_t i = openPos; i < content.size(); i++)
        {
            if (content[i] == '[') depth++;
            else if (content[i] == ']')
            {
                depth--;
                if (depth == 0) return i;
            }
        }
        return std::string::npos;
    }

    // Load all map data - MUST BE DEFINED BEFORE SaveToFile
    void LoadMapData(const std::string& helpPath)
    {
        fs::path absPath;
        if (fs::path(helpPath).is_relative())
        {
            char modulePath[MAX_PATH];
            GetModuleFileNameA(NULL, modulePath, MAX_PATH);
            fs::path exePath(modulePath);
            absPath = exePath.parent_path() / helpPath;
        }
        else
        {
            absPath = helpPath;
        }

        if (!fs::exists(absPath))
        {
            LOG_WARNING("GrenadeHelper", "Folder not found: {}", absPath.string());
            return;
        }

        for (const auto& entry : fs::directory_iterator(absPath))
        {
            if (entry.path().extension() == ".json")
            {
                std::string mapName = entry.path().stem().string();
                
                try
                {
                    std::ifstream file(entry.path());
                    if (!file.is_open()) continue;

                    std::string content((std::istreambuf_iterator<char>(file)),
                                        std::istreambuf_iterator<char>());
                    file.close();

                    MapData mapData;
                    mapData.MapName = mapName;

                    // Find "infos" array
                    size_t infosPos = content.find("\"infos\"");
                    if (infosPos == std::string::npos) continue;

                    size_t arrayStart = content.find("[", infosPos);
                    size_t arrayEnd = FindMatchingBracket(content, arrayStart);
                    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) continue;

                    // Find each object in array
                    size_t pos = arrayStart;
                    int throwCount = 0;
                    
                    while (true)
                    {
                        size_t objStart = content.find("{", pos);
                        if (objStart == std::string::npos || objStart > arrayEnd) break;
                        
                        size_t objEnd = content.find("}", objStart);
                        if (objEnd == std::string::npos || objEnd > arrayEnd) break;
                        
                        // Extract data from this object
                        ThrowPos t;
                        t.Name = ExtractString(content, objStart, "Name");
                        t.Style = ExtractInt(content, objStart, "Style");
                        t.Type = ExtractInt(content, objStart, "Type");
                        
                        std::vector<float> posArr = ExtractArray(content, objStart, "Position");
                        if (posArr.size() >= 3) {
                            t.Position.x = posArr[0];
                            t.Position.y = posArr[1];
                            t.Position.z = posArr[2];
                        }
                        
                        std::vector<float> angleArr = ExtractArray(content, objStart, "Angle");
                        if (angleArr.size() >= 2) {
                            t.Pitch = angleArr[0];
                            t.Yaw = angleArr[1];
                        }
                        
                        mapData.Throws.push_back(t);
                        throwCount++;
                        
                        pos = objEnd + 1;
                    }

                    MapsData[mapName] = mapData;
                    LOG_INFO("GrenadeHelper", "Loaded {} with {} throws", mapName, throwCount);
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR("GrenadeHelper", "Error loading {}: {}", mapName, e.what());
                }
            }
        }
        
        LOG_INFO("GrenadeHelper", "Total maps loaded: {}", MapsData.size());
    }

    // Update current map - MUST BE DEFINED BEFORE SaveToFile
    void UpdateMap(const std::string& mapName)
    {
        // Skip if map name is empty to avoid unnecessary logs
        if (mapName.empty()) {
            return;
        }
        
        CurrentMap = mapName;
        
        std::string cleanMapName = mapName;
        
        if (cleanMapName.find(".json") != std::string::npos)
            cleanMapName = cleanMapName.substr(0, cleanMapName.find(".json"));
        
        if (cleanMapName.find("maps/") == 0)
            cleanMapName = cleanMapName.substr(5);
        
        if (cleanMapName.find(".bsp") != std::string::npos)
            cleanMapName = cleanMapName.substr(0, cleanMapName.find(".bsp"));
        
        if (cleanMapName.find(".vpk") != std::string::npos)
            cleanMapName = cleanMapName.substr(0, cleanMapName.find(".vpk"));

        // Resolve practice/workshop variants (e.g. prac_dust2 -> de_dust2)
        if (MapsData.find(cleanMapName) == MapsData.end()) {
            for (const auto& pair : MapsData) {
                const std::string& knownName = pair.first;
                if (knownName.find("de_") == 0) {
                    std::string suffix = knownName.substr(3);
                    if (cleanMapName.length() >= suffix.length() &&
                        cleanMapName.compare(cleanMapName.length() - suffix.length(), suffix.length(), suffix) == 0) {
                        LOG_INFO("GrenadeHelper", "Resolved '{}' -> '{}'", cleanMapName, knownName);
                        cleanMapName = knownName;
                        break;
                    }
                }
            }
        }

        if (MapsData.find(cleanMapName) != MapsData.end())
        {
            CurrentThrows = &MapsData[cleanMapName].Throws;
            LOG_INFO("GrenadeHelper", "Map set to {} with {} throws", cleanMapName, CurrentThrows->size());
        }
        else
        {
            CurrentThrows = nullptr;
        }
    }

    // Record current position and angle
    void RecordPosition(const CEntity& LocalPlayer)
    {
        // Prevent rapid repeated recording (500ms cooldown)
        auto now = std::chrono::steady_clock::now();
        if (now - LastRecordTime < std::chrono::milliseconds(500))
            return;
        LastRecordTime = now;

        PendingThrow pt;
        pt.Position = LocalPlayer.Pawn.Pos;
        pt.Pitch = LocalPlayer.Pawn.ViewAngle.x;
        pt.Yaw = LocalPlayer.Pawn.ViewAngle.y;

        // Use default type if weapon detection fails (-1)
        int detectedType = GetGrenadeTypeFromWeapon(LocalPlayer.Pawn.WeaponName);
        pt.Type = (detectedType >= 0) ? detectedType : DefaultGrenadeType;

        pt.Style = DefaultThrowStyle;

        // Auto-generate name
        ThrowCounter++;
        char autoName[64];
        const char* typeName = "Unknown";
        switch (pt.Type) {
            case 0: typeName = "Flash"; break;
            case 1: typeName = "Smoke"; break;
            case 2: typeName = "HE"; break;
            case 3: typeName = "Molotov"; break;
        }
        sprintf_s(autoName, "Throw_%d_%s", ThrowCounter, typeName);
        pt.Name = autoName;
        pt.Named = true;  // Mark as named for auto-save

        // Generate timestamp
        auto t = std::time(nullptr);
        struct tm timeinfo;
        localtime_s(&timeinfo, &t);
        std::stringstream ss;
        ss << std::put_time(&timeinfo, "%H:%M:%S");
        pt.Timestamp = ss.str();

        PendingThrows.push_back(pt);
        LOG_INFO("GrenadeHelper", "Recorded: {} at ({}, {}, {}) Angle: ({}, {})",
                 pt.Name, pt.Position.x, pt.Position.y, pt.Position.z, pt.Pitch, pt.Yaw);

        // Set save flag for auto-save (will be handled in render loop)
        if (AutoSave && !CurrentMap.empty()) {
            NeedSave = true;
        }
    }

    // Name a pending throw and save it
    void NamePendingThrow(int index, const std::string& name, int style, int type)
    {
        if (index < 0 || index >= (int)PendingThrows.size())
            return;

        PendingThrows[index].Name = name;
        PendingThrows[index].Style = style;
        PendingThrows[index].Type = type;
        PendingThrows[index].Named = true;

    }

    // Save named throws to map file
    void SaveToFile(const std::string& mapName)
    {
        if (mapName.empty()) {
            LOG_WARNING("GrenadeHelper", "No map name, cannot save");
            return;
        }

        // Clean map name (remove maps/, .vpk, .bsp, .json)
        std::string cleanMapName = mapName;
        if (cleanMapName.find("maps/") == 0)
            cleanMapName = cleanMapName.substr(5);
        if (cleanMapName.find(".vpk") != std::string::npos)
            cleanMapName = cleanMapName.substr(0, cleanMapName.find(".vpk"));
        if (cleanMapName.find(".bsp") != std::string::npos)
            cleanMapName = cleanMapName.substr(0, cleanMapName.find(".bsp"));
        if (cleanMapName.find(".json") != std::string::npos)
            cleanMapName = cleanMapName.substr(0, cleanMapName.find(".json"));

        // Build file path
        fs::path absPath;
        char modulePath[MAX_PATH];
        GetModuleFileNameA(NULL, modulePath, MAX_PATH);
        fs::path exePath(modulePath);
        absPath = exePath.parent_path() / "data" / "grenade-helper" / (cleanMapName + ".json");

        // Create directory if not exists
        fs::path dirPath = exePath.parent_path() / "data" / "grenade-helper";
        if (!fs::exists(dirPath)) {
            fs::create_directory(dirPath);
        }

        // Use in-memory data if available
        std::vector<ThrowPos> throwsToSave;
        if (MapsData.find(cleanMapName) != MapsData.end()) {
            throwsToSave = MapsData[cleanMapName].Throws;
        }

        // Add named pending throws
        int namedCount = 0;
        for (const auto& pt : PendingThrows)
        {
            if (pt.Named && !pt.Name.empty())
            {
                ThrowPos t;
                t.Name = pt.Name;
                t.Style = pt.Style;
                t.Type = pt.Type;
                t.Position = pt.Position;
                t.Pitch = pt.Pitch;
                t.Yaw = pt.Yaw;
                throwsToSave.push_back(t);
                namedCount++;
            }
        }

        // Write to file
        std::ofstream outFile(absPath);
        if (!outFile.is_open()) {
            LOG_ERROR("GrenadeHelper", "Failed to open file for writing: {}", absPath.string());
            return;
        }

        outFile << "{\n\t\"infos\":[";
        for (size_t i = 0; i < throwsToSave.size(); i++)
        {
            const auto& t = throwsToSave[i];
            outFile << "{\n";
            outFile << "\t\t\"Name\":\"" << t.Name << "\",\n";
            outFile << "\t\t\"Style\":" << t.Style << ",\n";
            outFile << "\t\t\"Type\":" << t.Type << ",\n";
            outFile << "\t\t\"Position\":\[" << std::fixed << std::setprecision(6)
                    << t.Position.x << "," << t.Position.y << "," << t.Position.z << "],\n";
            outFile << "\t\t\"Angle\":\[" << t.Pitch << "," << t.Yaw << "]\n";
            outFile << "\t}";
            if (i < throwsToSave.size() - 1)
                outFile << ",";
        }
        outFile << "]\n}\n";
        outFile.close();

        LOG_INFO("GrenadeHelper", "Saved {} throws to {}", throwsToSave.size(), absPath.string());

        // Clear saved pending throws
        PendingThrows.erase(
            std::remove_if(PendingThrows.begin(), PendingThrows.end(),
                [](const PendingThrow& pt) { return pt.Named; }),
            PendingThrows.end()
        );

        // Reload map data
        LoadMapData("data/grenade-helper");
        UpdateMap(mapName);
    }

    // Delete a pending throw
    void DeletePendingThrow(int index)
    {
        if (index >= 0 && index < (int)PendingThrows.size())
        {
            PendingThrows.erase(PendingThrows.begin() + index);
        }
    }

    // Clear all pending throws
    void ClearPendingThrows()
    {
        PendingThrows.clear();
    }

    // Calculate distance
    float GetDistance(const Vec3& a, const Vec3& b)
    {
        return sqrtf(powf(b.x - a.x, 2) + powf(b.y - a.y, 2) + powf(b.z - a.z, 2));
    }

    // Convert pitch/yaw angles to a unit direction vector (CS2 convention)
    Vec3 AngleToDirection(float pitch, float yaw)
    {
        float pitchRad = pitch * 3.14159265f / 180.0f;
        float yawRad   = yaw   * 3.14159265f / 180.0f;

        Vec3 dir;
        dir.x =  cosf(yawRad) * cosf(pitchRad);
        dir.y =  sinf(yawRad) * cosf(pitchRad);
        dir.z = -sinf(pitchRad);  // Negative pitch = up
        return dir;
    }

    // Project a DIRECTION to screen coordinates using the view matrix.
    // This is position-independent: it tells you where a direction appears
    // on your screen regardless of where you stand.
    // Mathematically equivalent to WorldToScreen of a point at infinity.
    bool DirectionToScreen(const Vec3& dir, Vec2& screenPos)
    {
        const auto& M = gGame.View.Matrix;

        // Perspective divide factor (only rotational part, no translation)
        float w = M[3][0] * dir.x + M[3][1] * dir.y + M[3][2] * dir.z;
        if (w <= 0.001f)
            return false;  // Direction is behind the camera

        float halfW = Gui.Window.Size.x / 2.0f;
        float halfH = Gui.Window.Size.y / 2.0f;

        screenPos.x = halfW + (M[0][0] * dir.x + M[0][1] * dir.y + M[0][2] * dir.z) / w * halfW;
        screenPos.y = halfH - (M[1][0] * dir.x + M[1][1] * dir.y + M[1][2] * dir.z) / w * halfH;
        return true;
    }

    // Draw an arrow pointing towards a target direction
    void DrawDirectionArrow(ImDrawList* drawList, Vec2 screenCenter, Vec2 targetScreen, ImColor color, float distance)
    {
        // Calculate direction from screen center to target
        float dx = targetScreen.x - screenCenter.x;
        float dy = targetScreen.y - screenCenter.y;
        float len = sqrtf(dx * dx + dy * dy);
        
        if (len < 1.0f) return;
        
        // Normalize direction
        float nx = dx / len;
        float ny = dy / len;
        
        // Arrow position: at edge of screen, pointing towards target
        // Place arrow at a fixed distance from center
        float arrowDist = 160.0f;  // Distance from screen center
        Vec2 arrowPos;
        arrowPos.x = screenCenter.x + nx * arrowDist;
        arrowPos.y = screenCenter.y + ny * arrowDist;
        
        // Arrow size
        float arrowSize = 50.0f;
        
        // Draw arrow (triangle pointing in direction of target)
        // Arrow tip
        Vec2 tip;
        tip.x = arrowPos.x + nx * arrowSize;
        tip.y = arrowPos.y + ny * arrowSize;
        
        // Arrow base corners (perpendicular to direction)
        float perpX = -ny;  // Perpendicular vector
        float perpY = nx;
        
        Vec2 base1, base2;
        base1.x = arrowPos.x + perpX * (arrowSize * 0.4f);
        base1.y = arrowPos.y + perpY * (arrowSize * 0.4f);
        base2.x = arrowPos.x - perpX * (arrowSize * 0.4f);
        base2.y = arrowPos.y - perpY * (arrowSize * 0.4f);
        
        // Draw filled triangle
        drawList->AddTriangleFilled(
            ImVec2(tip.x, tip.y),
            ImVec2(base1.x, base1.y),
            ImVec2(base2.x, base2.y),
            color
        );
        
        // Draw outline
        drawList->AddTriangle(
            ImVec2(tip.x, tip.y),
            ImVec2(base1.x, base1.y),
            ImVec2(base2.x, base2.y),
            ImColor(0, 0, 0, 200), 3.0f
        );
    }

    // Draw throw position on screen
    void DrawThrowPos(const ThrowPos& t, const CEntity& LocalPlayer, ImDrawList* drawList)
    {
        Vec3 playerPos = LocalPlayer.Pawn.Pos;
        float dist = GetDistance(playerPos, t.Position);

        if (dist > MaxDistance)
            return;

        Vec2 screenPos;
        if (!gGame.View.WorldToScreen(t.Position, screenPos))
            return;

        float alpha = 1.0f - (dist / MaxDistance) * 0.5f;
        ImColor color = GetGrenadeColor(t.Type);
        color.Value.w = alpha;

        // Draw box
        if (ShowBox)
        {
            ImVec2 min(screenPos.x - BoxSize, screenPos.y - BoxSize);
            ImVec2 max(screenPos.x + BoxSize, screenPos.y + BoxSize);
            drawList->AddRect(min, max, color, 2.0f, 0, 2.0f);
        }

        // Draw name and angle info
        if (ShowName)
        {
            // Get style name
            const char* styleName = "Stand";
            switch (t.Style) {
                case 1: styleName = "Run"; break;
                case 2: styleName = "Jump"; break;
                case 3: styleName = "Crouch"; break;
                case 4: styleName = "Run+Jump"; break;
            }

            char text[128];
            sprintf_s(text, "%s [%s] P:%.0f Y:%.0f", t.Name.c_str(), styleName, t.Pitch, t.Yaw);
            drawList->AddText(ImVec2(screenPos.x - BoxSize, screenPos.y + BoxSize + 2),
                             ImColor(255, 255, 255, (int)(alpha * 255)), text);
        }

        // Draw distance
        char distText[32];
        sprintf_s(distText, "%.2fm", dist / 100.0f);
        drawList->AddText(ImVec2(screenPos.x - BoxSize, screenPos.y - BoxSize - 15), 
                         ImColor(200, 200, 200, (int)(alpha * 255)), distText);

        // Draw line to player
        if (ShowLine)
        {
            Vec2 playerScreen;
            if (gGame.View.WorldToScreen(playerPos, playerScreen))
            {
                drawList->AddLine(ImVec2(screenPos.x, screenPos.y),
                                 ImVec2(playerScreen.x, playerScreen.y),
                                 ImColor(color.Value.x, color.Value.y, color.Value.z, alpha * 0.3f), 1.0f);
            }
        }
    }

    // Get grenade type name
    const char* GetGrenadeTypeName(int type)
    {
        switch (type) {
            case FLASH: return lang.grenade_typename_flash.c_str();
            case SMOKE: return lang.grenade_typename_smoke.c_str();
            case HE: return lang.grenade_typename_he.c_str();
            case MOLOTOV: return lang.grenade_typename_fire.c_str();
            default: return lang.grenade_typename_unknown.c_str();
        }
    }

    // Get style name
    const char* GetStyleName(int style)
    {
        switch (style) {
            case 0: return lang.grenade_stylename_stand.c_str();
            case 1: return lang.grenade_stylename_run.c_str();
            case 2: return lang.grenade_stylename_jump.c_str();
            case 3: return lang.grenade_stylename_crouch.c_str();
            case 4: return lang.grenade_stylename_runjump.c_str();
            default: return lang.grenade_stylename_unknown.c_str();
        }
    }

    // Render all throw positions
    void Render(const CEntity& LocalPlayer)
    {
        if (!Enabled) {
            return;
        }
        
        if (!CurrentThrows) {
            return;
        }
        
        if (CurrentThrows->empty()) {
            return;
        }

        // Get current weapon type - only show if holding a grenade
        GrenadeType currentGrenade = GetGrenadeTypeFromWeapon(LocalPlayer.Pawn.WeaponName);
        if (currentGrenade == UNKNOWN) {
            return;  // Not holding a grenade, don't show anything
        }

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        // Find nearest throw position of the same type within AimIndicatorDistance
        const ThrowPos* nearestThrow = nullptr;
        float nearestDist = AimIndicatorDistance;
        
        for (const auto& t : *CurrentThrows)
        {
            if (t.Type != currentGrenade)
                continue;
            
            float dist = GetDistance(LocalPlayer.Pawn.Pos, t.Position);
            if (dist < nearestDist) {
                nearestDist = dist;
                nearestThrow = &t;
            }
        }
        
        // Draw all throw positions of same grenade type (boxes/names)
        for (const auto& t : *CurrentThrows)
        {
            if (t.Type != currentGrenade)
                continue;
                
            DrawThrowPos(t, LocalPlayer, drawList);
        }
        
        // Draw aim target ONLY for nearest throw position within AimIndicatorDistance
        if (nearestThrow && nearestDist < AimIndicatorDistance)
        {
            // Display throw info at top of screen
            ImVec2 displaySize = ImGui::GetIO().DisplaySize;

            // Get grenade type and style names
            const char* grenadeTypeName = GetGrenadeTypeName(nearestThrow->Type);
            const char* styleName = GetStyleName(nearestThrow->Style);
            ImColor grenadeColor = GetGrenadeColor(nearestThrow->Type);

            // Draw info text at top center with larger font
            char infoText[256];
            sprintf_s(infoText, "[%s] %s - %s", grenadeTypeName, nearestThrow->Name.c_str(), styleName);

            // Use larger font size
            float fontSize = 32.0f;  // Larger font
            ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, infoText);
            float textX = (displaySize.x - textSize.x) / 2.0f;
            float textY = 120.0f;  // Moved down from 50 to 120 pixels from top

            // Draw text with larger font in green
            drawList->AddText(ImGui::GetFont(), fontSize, ImVec2(textX, textY), ImColor(0, 255, 0, 240), infoText);
            
            // Calculate movement direction to throw position
            Vec3 playerPos = LocalPlayer.Pawn.Pos;
            Vec3 toTarget;
            toTarget.x = nearestThrow->Position.x - playerPos.x;
            toTarget.y = nearestThrow->Position.y - playerPos.y;
            toTarget.z = nearestThrow->Position.z - playerPos.z;
            
            // Get player's view angle (yaw)
            float playerYaw = LocalPlayer.Pawn.ViewAngle.y;
            float yawRad = playerYaw * 3.14159265f / 180.0f;

            // Convert world direction to local direction (relative to player's view)
            // Forward direction based on yaw
            float forwardX = cos(yawRad);
            float forwardY = sin(yawRad);
            // Right direction (perpendicular to forward, clockwise rotation)
            float rightX = forwardY;
            float rightY = -forwardX;
            
            // Project movement vector onto forward and right
            float forwardDist = toTarget.x * forwardX + toTarget.y * forwardY;
            float rightDist = toTarget.x * rightX + toTarget.y * rightY;
            
            // Build movement direction text
            char moveText[128] = "";
            char* p = moveText;
            
            // Forward/Backward
            if (fabsf(forwardDist) > 1.0f) {  // More than 1 unit
                if (forwardDist > 0) {
                    p += sprintf_s(p, 128 - (p - moveText), "%s %.2fm ", lang.dir_forward.c_str(), forwardDist / 100.0f);
                } else {
                    p += sprintf_s(p, 128 - (p - moveText), "%s %.2fm ", lang.dir_back.c_str(), -forwardDist / 100.0f);
                }
            }
            
            // Left/Right
            if (fabsf(rightDist) > 1.0f) {  // More than 1 unit
                if (rightDist > 0) {
                    p += sprintf_s(p, 128 - (p - moveText), "%s %.2fm ", lang.dir_right.c_str(), rightDist / 100.0f);
                } else {
                    p += sprintf_s(p, 128 - (p - moveText), "%s %.2fm ", lang.dir_left.c_str(), -rightDist / 100.0f);
                }
            }
            
            // Vertical
            if (fabsf(toTarget.z) > 1.0f) {
                if (toTarget.z > 0) {
                    p += sprintf_s(p, 128 - (p - moveText), "%s %.2fm", lang.dir_up.c_str(), toTarget.z / 100.0f);
                } else {
                    p += sprintf_s(p, 128 - (p - moveText), "%s %.2fm", lang.dir_down.c_str(), -toTarget.z / 100.0f);
                }
            }
            
            // Draw movement direction indicator (compass style)
            Vec2 screenCenter(displaySize.x / 2.0f, displaySize.y / 2.0f);

            // Calculate movement direction in screen space
            // forwardDist = forward movement (positive = forward, negative = back)
            // rightDist = right movement (positive = right, negative = left)
            float moveX = rightDist;   // Left/Right
            float moveY = -forwardDist; // Forward/Back (inverted for screen Y)

            float moveLen = sqrtf(moveX * moveX + moveY * moveY);
            float horizontalDist = sqrtf(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

            // Draw compass-style indicator at screen center (always show)
            {

                // Indicator position: below screen center
                Vec2 indicatorPos;
                indicatorPos.x = screenCenter.x;
                indicatorPos.y = screenCenter.y + 150.0f;

                // Draw outer ring (dark background)
                float ringRadius = 35.0f;
                drawList->AddCircle(ImVec2(indicatorPos.x, indicatorPos.y), ringRadius,
                                   ImColor(50, 50, 50, 200), 32, 3.0f);
                drawList->AddCircleFilled(ImVec2(indicatorPos.x, indicatorPos.y), ringRadius - 1,
                                         ImColor(20, 20, 20, 180), 32);

                // Draw direction arrow inside ring
                if (moveLen > 1.0f) {
                    float nx = moveX / moveLen;
                    float ny = moveY / moveLen;

                    // Arrow size fixed (always at ring edge)
                    float arrowLen = ringRadius - 8.0f;
                    float arrowWidth = 14.0f;

                    // Arrow tip
                    Vec2 tip;
                    tip.x = indicatorPos.x + nx * arrowLen;
                    tip.y = indicatorPos.y + ny * arrowLen;

                    // Arrow base corners
                    float perpX = -ny;
                    float perpY = nx;

                    Vec2 base1, base2;
                    base1.x = indicatorPos.x + perpX * arrowWidth * 0.5f;
                    base1.y = indicatorPos.y + perpY * arrowWidth * 0.5f;
                    base2.x = indicatorPos.x - perpX * arrowWidth * 0.5f;
                    base2.y = indicatorPos.y - perpY * arrowWidth * 0.5f;

                    // Color based on distance (red=far, yellow=medium, green=close)
                    ImColor arrowColor;
                    if (horizontalDist > 200.0f) {
                        arrowColor = ImColor(255, 80, 80, 255);  // Red - far
                    } else if (horizontalDist > 50.0f) {
                        arrowColor = ImColor(255, 255, 0, 255);  // Yellow - medium
                    } else {
                        arrowColor = ImColor(0, 255, 100, 255);  // Green - close
                    }

                    // Draw filled arrow
                    drawList->AddTriangleFilled(
                        ImVec2(tip.x, tip.y),
                        ImVec2(base1.x, base1.y),
                        ImVec2(base2.x, base2.y),
                        arrowColor
                    );

                    // Draw outline
                    drawList->AddTriangle(
                        ImVec2(tip.x, tip.y),
                        ImVec2(base1.x, base1.y),
                        ImVec2(base2.x, base2.y),
                        ImColor(0, 0, 0, 200), 2.5f
                    );
                }

                // Draw distance text in center of ring
                char distText[32];
                sprintf_s(distText, "%.2fm", horizontalDist / 100.0f);
                ImVec2 distTextSize = ImGui::GetFont()->CalcTextSizeA(16.0f, FLT_MAX, 0.0f, distText);
                drawList->AddText(ImGui::GetFont(), 16.0f,
                                 ImVec2(indicatorPos.x - distTextSize.x / 2.0f,
                                        indicatorPos.y - distTextSize.y / 2.0f),
                                 ImColor(255, 255, 255, 255), distText);

                // Draw direction labels around ring
                auto drawLabel = [&](const char* text, float angle, ImColor color) {
                    float labelDist = ringRadius + 12.0f;
                    float rad = angle * 3.14159f / 180.0f;
                    float lx = indicatorPos.x + sinf(rad) * labelDist;
                    float ly = indicatorPos.y - cosf(rad) * labelDist;
                    ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(14.0f, FLT_MAX, 0.0f, text);
                    drawList->AddText(ImGui::GetFont(), 14.0f,
                                     ImVec2(lx - textSize.x / 2.0f, ly - textSize.y / 2.0f),
                                     color, text);
                };

                drawLabel(lang.dir_f.c_str(), 0.0f, ImColor(255, 255, 255, 200));
                drawLabel(lang.dir_b.c_str(), 180.0f, ImColor(255, 255, 255, 200));
                drawLabel(lang.dir_l.c_str(), 270.0f, ImColor(255, 255, 255, 200));
                drawLabel(lang.dir_r.c_str(), 90.0f, ImColor(255, 255, 255, 200));
            }
            
            // Project saved aim direction directly to screen (position-independent)
            Vec3 aimDir = AngleToDirection(nearestThrow->Pitch, nearestThrow->Yaw);
            Vec2 aimScreen;
            if (DirectionToScreen(aimDir, aimScreen))
            {
                // Get screen center (player's crosshair position)
                Vec2 screenCenter(displaySize.x / 2.0f, displaySize.y / 2.0f);
                
                // Calculate distance from screen center to aim point
                float screenDist = sqrtf(powf(aimScreen.x - screenCenter.x, 2) + 
                                        powf(aimScreen.y - screenCenter.y, 2));
                
                // Draw aim crosshair with ring + broken cross lines
                float crossLen = 20.0f;
                float crossGap = 4.0f;
                float ringRadius = 24.0f;
                float thickness = 2.0f;
                ImColor aimColor(255, 0, 0, 240);
                ImColor outlineColor(0, 0, 0, 180);
                
                // Outer ring
                drawList->AddCircle(ImVec2(aimScreen.x, aimScreen.y), ringRadius,
                                   outlineColor, 32, thickness + 2.0f);
                drawList->AddCircle(ImVec2(aimScreen.x, aimScreen.y), ringRadius,
                                   aimColor, 32, thickness);
                
                // Horizontal lines (with gap in center) + outline
                auto drawThickLine = [&](ImVec2 a, ImVec2 b) {
                    drawList->AddLine(a, b, outlineColor, thickness + 2.0f);
                    drawList->AddLine(a, b, aimColor, thickness);
                };
                drawThickLine(ImVec2(aimScreen.x - crossLen, aimScreen.y),
                              ImVec2(aimScreen.x - crossGap, aimScreen.y));
                drawThickLine(ImVec2(aimScreen.x + crossGap, aimScreen.y),
                              ImVec2(aimScreen.x + crossLen, aimScreen.y));
                // Vertical lines (with gap in center)
                drawThickLine(ImVec2(aimScreen.x, aimScreen.y - crossLen),
                              ImVec2(aimScreen.x, aimScreen.y - crossGap));
                drawThickLine(ImVec2(aimScreen.x, aimScreen.y + crossGap),
                              ImVec2(aimScreen.x, aimScreen.y + crossLen));
                
                // Draw direction arrow only when aim point is far from crosshair
                float arrowHideDistance = 60.0f;  // Hide arrow when within this distance
                if (screenDist > arrowHideDistance)
                {
                    ImColor arrowColor(255, 255, 0, 200);  // Yellow arrow
                    DrawDirectionArrow(drawList, screenCenter, aimScreen, arrowColor, screenDist);
                }
            }
        }
    }

    // Get available maps
    std::vector<std::string> GetAvailableMaps()
    {
        std::vector<std::string> maps;
        for (const auto& pair : MapsData)
            maps.push_back(pair.first);
        return maps;
    }

    // Delete a throw from current map data
    void DeleteThrow(const std::string& mapName, int index)
    {
        if (MapsData.find(mapName) == MapsData.end())
            return;
        
        auto& throws = MapsData[mapName].Throws;
        if (index >= 0 && index < (int)throws.size())
        {
            throws.erase(throws.begin() + index);
            
            // Update current throws pointer
            if (CurrentThrows && CurrentMap == mapName)
            {
                CurrentThrows = &MapsData[mapName].Throws;
            }
            
            // Save to file
            SaveToFile(mapName);
        }
    }

    // Update a throw in current map data
    void UpdateThrow(const std::string& mapName, int index, const std::string& name, int style, int type,
                            float posX, float posY, float posZ, float pitch, float yaw)
    {
        if (MapsData.find(mapName) == MapsData.end())
            return;
        
        auto& throws = MapsData[mapName].Throws;
        if (index >= 0 && index < (int)throws.size())
        {
            throws[index].Name = name;
            throws[index].Style = style;
            throws[index].Type = type;
            throws[index].Position.x = posX;
            throws[index].Position.y = posY;
            throws[index].Position.z = posZ;
            throws[index].Pitch = pitch;
            throws[index].Yaw = yaw;
            
            // Update current throws pointer
            if (CurrentThrows && CurrentMap == mapName)
            {
                CurrentThrows = &MapsData[mapName].Throws;
            }
            
            // Save to file
            SaveToFile(mapName);
        }
    }

    // Get throw by index (for editing)
    ThrowPos* GetThrow(const std::string& mapName, int index)
    {
        if (MapsData.find(mapName) == MapsData.end())
            return nullptr;
        
        auto& throws = MapsData[mapName].Throws;
        if (index >= 0 && index < (int)throws.size())
            return &throws[index];
        
        return nullptr;
    }
}
