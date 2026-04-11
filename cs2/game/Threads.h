#pragma once

#include "AppState.h"

#include "../render/Cheats.h"

#include <array>

// Connection state machine: search game, init addresses, detect game loss
VOID ConnectionThread();

// Merged data pipeline: matrix + local player + entities + scatter + weapon names
VOID DataThread();

// Low-frequency: entity list base address + map name
VOID SlowUpdateThread();

// Keyboard state polling
VOID KeysCheckThread();

// Web Radar: WebSocket server broadcasting game data to browser clients
VOID WebRadarThread();
