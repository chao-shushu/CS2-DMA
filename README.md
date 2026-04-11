**English** | **[中文](README_CN.md)**

# CS2-DMA
![3feefde8e0a9558fad9e715bdb83c18c](https://github.com/user-attachments/assets/5f5adad4-4aa0-4f44-888e-cc24f6bc5231)


An external CS2 (Counter-Strike 2) tool built with C++, using DMA (Direct Memory Access) hardware to read game memory via FPGA devices and render ESP, radar, grenade helper, and more on a separate machine. This project does not and will never include aimbot-related features via kmbox or similar devices.

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Visual Studio](https://img.shields.io/badge/IDE-Visual%20Studio%202026-5C2D91?logo=visual-studio&logoColor=white)
![Platform](https://img.shields.io/badge/platform-Windows%20x64-0078D6)
![License](https://img.shields.io/badge/license-MIT-green)

> ⭐ If you like this project, please give it a Star to support continued development!

---

## Features

### Visuals / ESP
- **Box ESP** — Normal / Dynamic / Corner styles with fill, rounding, and thickness options
- **Bone ESP** — Full skeleton rendering
- **Health Bar / Armor Bar** — Horizontal or vertical, with damage fallback animation
- **Weapon ESP** — Displays currently held weapon name
- **Distance ESP** — Shows distance to enemies
- **Player Name** — Displays player nicknames
- **Eye Ray** — Shows enemy view direction
- **Snaplines** — Lines from screen edge to enemies
- **Head Dot** — Head position marker
- **Safe Zone** — Crosshair area ESP cutout to reduce visual clutter
<img width="827" height="524" alt="image" src="https://github.com/user-attachments/assets/6589c93a-007a-467f-966b-6b6126f92d60" />

### Bomb ESP
- Planted / Carried / Dropped / Defusing status display
- Explosion and defuse countdown timers
<img width="837" height="517" alt="image" src="https://github.com/user-attachments/assets/8f7d1785-b21e-41a5-80e2-ebd7e122f4aa" />

### Projectile ESP
- Real-time display of in-flight flashbangs, smokes, HE grenades, molotovs, and decoys
- Explosion/effect radius circle rendering
<img width="838" height="523" alt="image" src="https://github.com/user-attachments/assets/79e82266-5a15-4029-ab8e-b7b8fa647302" />

### Web Radar
- Built-in WebSocket server
- Uses [cs2_webradar](https://github.com/clauadv/cs2_webradar) frontend
- Any LAN device can view real-time radar via browser
<img width="840" height="517" alt="image" src="https://github.com/user-attachments/assets/6dbaf41a-4dfd-41f8-85e4-8fa2d9b1a52f" />
<img width="773" height="457" alt="image" src="https://github.com/user-attachments/assets/5fb4025a-d06b-4f2e-b81f-ba9b55281252" />

### Grenade Helper
- Per-map preset throw positions (JSON format)
- Real-time direction arrow + distance indicator
- Record / edit / delete custom positions
- Supports flash, smoke, HE, and molotov types
<img width="1615" height="1031" alt="image" src="https://github.com/user-attachments/assets/9fb5df0e-9699-4a5a-8253-ca65b0d2aad5" />

### DMA Low-Latency Optimization
- **Scatter Batch Reads** — All entity data is merged into a single DMA operation, eliminating per-read PCIe round-trip latency
- **On-Demand Reading** — Only reads fields required by currently enabled features; the entire data pipeline sleeps when no features are active — zero wasted transfers
- **Tiered Entity Caching** — High-frequency data (position, health) is read every frame; low-frequency data (name, team) updates at 5/50 frame intervals, reducing 80%+ redundant reads
- **Zero-Copy Snapshot** — `DataThread` holds a write lock only briefly during pointer swap; render thread reads without blocking — data latency < 1 frame

### Other
- **Config System** — Create / save / load / delete multiple configs, auto-loads `_autosave.config` on startup
- **Multi-language** — Chinese / English toggle
- **Logging System** — Leveled logging (TRACE → FATAL) with ring buffer for crash diagnostics
- **Crash Handler** — SEH + `std::terminate` capture, auto-generates `.log` + `.dmp` with recent logs, feature state, and system info

---

## Quick Start

### Download

Go to the [Releases](https://github.com/chao-shushu/CS2-DMA/releases) page and download the latest `CS2-DMA-Release.zip`.

### Directory Structure After Extraction

```
CS2-DMA/
├── cs2.exe              # Main executable
├── vmm.dll              # MemProcFS core library
├── leechcore.dll        # LeechCore device communication
├── FTD3XX.dll           # FTDI USB3 driver
├── data/
│   ├── offsets.json     # Game offsets
│   ├── client_dll.json  # client.dll offsets
│   └── grenade-helper/  # Grenade helper map data
├── saved/configs/       # Config storage (auto-generated)
└── logs/                # Log directory (auto-generated)
```

### How to Run

1. **Connect FPGA device** — Ensure your DMA hardware is properly connected to the secondary machine
2. **Launch CS2 on the main machine** — Open the game and join a match
3. **Run `cs2.exe` on the secondary machine** — The program will automatically:
   - Initialize DMA connection
   - Search for the `cs2.exe` process
   - Start rendering ESP once the game is detected
4. **Press `F8` to open the menu** — Toggle features on/off from the menu

### Menu Tabs

| Tab | Description |
|-----|-------------|
| **Visuals** | Box, bone, health bar, armor bar, weapon, distance, name, eye ray, snaplines, etc. |
| **Radar** | Web Radar toggle, port, broadcast interval |
| **Grenade** | Grenade helper toggle, record positions, edit/delete |
| **Settings** | Frame rate limit, VSync, language, team filter |
| **Config** | Create, save, load, delete config files |

### Offsets Outdated?

After each CS2 update, game offsets may become invalid, causing ESP to not display or show incorrect data. To fix:

1. Get the latest `offsets.json` and `client_dll.json` from [cs2-dumper](https://github.com/a2x/cs2-dumper)
2. Replace the corresponding files in the `data/` directory
3. Restart the program

> If you're building from source, you can also use the `tools/update-offsets.ps1` script to update automatically.

---

## Bug Reports

Found an issue? Please submit a bug report via [GitHub Issues](https://github.com/chao-shushu/CS2-DMA/issues).

### What to Include in Your Issue

1. **Problem description** — Briefly describe the issue
2. **Steps to reproduce** — How to trigger the bug
3. **Log file** — The latest `.log` file from `logs/`
4. **Crash dump** (if the program crashed) — `crash_*.log` and `crash_*.dmp` files from `logs/`
5. **Environment info**:
   - Windows version (e.g. Win11 24H2)
   - FPGA device model
   - Whether CS2 was recently updated (are offsets up to date?)

### Where Are the Log Files?

The program automatically generates files in the `logs/` directory:
- `cs2dma_YYYYMMDD_HHMMSS.log` — Runtime log
- `crash_YYYYMMDD_HHMMSS.log` — Crash diagnostic report (includes recent logs, feature state, system info)
- `crash_YYYYMMDD_HHMMSS.dmp` — MiniDump file

> Attaching these files to your issue helps resolve problems much faster.

---

## Project Structure

```
CS2-DMA/
├── cs2/                        # Main project source
│   ├── main.cpp                # Entry: init logging, DMA, threads, render window
│   ├── game/                   # Game logic layer
│   │   ├── Threads.cpp/h       # Thread definitions (Connection / Data / SlowUpdate / Keys / WebRadar)
│   │   ├── Entity.cpp/h        # Entity data structures and reading
│   │   ├── Bone.cpp/h          # Bone definitions and parsing
│   │   ├── Game.cpp/h          # Game address initialization
│   │   ├── Offsets.cpp/h       # Offset parsing (dynamically loaded from JSON)
│   │   ├── GlobalVars.cpp/h    # Global variable reading
│   │   ├── AppState.h          # Application state machine enum
│   │   ├── MenuConfig.h        # All menu config items (inline globals)
│   │   └── View.h              # View matrix
│   ├── render/                 # Rendering layer
│   │   ├── Cheats.cpp/h        # ESP main render entry + GameSnapshot definition
│   │   ├── Render.cpp/h        # Render utilities (box, bone, health bar, etc.)
│   │   ├── GUI.cpp/h           # ImGui menu interface
│   │   ├── GrenadeHelper.cpp/h # Grenade helper
│   │   └── WebRadar.cpp/h      # WebSocket server + radar data serialization
│   ├── config/                 # Config system
│   │   ├── ConfigSaver.cpp/h   # Config file read/write
│   │   ├── ConfigMenu.cpp/h    # Config menu UI
│   │   ├── SettingsManager.cpp/h # Global settings (language, etc.)
│   │   └── Language.h          # Multi-language strings
│   ├── utils/                  # Utility modules
│   │   ├── Logger.cpp/h        # Logging (singleton, thread-safe, ring buffer)
│   │   ├── CrashHandler.cpp/h  # Crash handler (MiniDump + diagnostic report)
│   │   ├── ProcessManager.h    # DMA memory read wrapper (VMMDLL)
│   │   └── base64.h            # Base64 utility
│   ├── includes/               # Third-party headers (vmmdll.h, leechcore.h, rapidjson)
│   ├── SDK/                    # VMMDLL library files (Lib/Include)
│   └── OS-ImGui/               # ImGui rendering framework wrapper
├── data/                       # Runtime data
│   ├── offsets.json            # Game offsets
│   ├── client_dll.json         # client.dll offsets
│   └── grenade-helper/         # Grenade helper map data (JSON)
├── saved/                      # User config storage
├── logs/                       # Logs and crash dumps
├── tools/                      # Automation scripts
│   ├── update-offsets.ps1      # Offset update script (supports local/DMA mode)
│   └── update-offsets.bat      # Batch entry point
├── external/                   # External tools
│   ├── dumper/                 # cs2-dumper (Rust, for obtaining offsets)
│   └── webradar/               # cs2_webradar frontend (React)
├── docs/                       # Documentation
│   ├── webradar-setup.md       # Web Radar deployment guide
│   ├── edit-history.md         # Development changelog
│   └── LICENSE                 # MIT License
└── dma.slnx                    # Visual Studio solution
```

---

## Build Guide

### Requirements

| Dependency | Version |
|------------|---------|
| Visual Studio | 2026 Community or later |
| C++ Standard | C++17 |
| Platform | x64 |
| Windows SDK | 10.0+ |

### Runtime Dependencies

The following DLLs must be in the same directory as `cs2.exe`:

| File | Description |
|------|-------------|
| `vmm.dll` | MemProcFS core library |
| `leechcore.dll` | LeechCore device communication layer |
| `FTD3XX.dll` | FTDI USB3 driver (required for FPGA devices) |

> These DLLs come from the [MemProcFS](https://github.com/ufrisk/MemProcFS) release package and are included in the repository.

### Build Steps

```powershell
# 1. Clone the repository
git clone https://github.com/chao-shushu/CS2-DMA.git
cd CS2-DMA

# 2. Open dma.slnx in Visual Studio, or build from command line:
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" `
    "dma.slnx" /p:Configuration=Release /p:Platform=x64 /t:Rebuild /m

# Build output: cs2.exe
```

### Updating Offsets

After game updates, offsets become invalid and need to be re-obtained:

```powershell
# Method 1: On the machine running CS2 (local mode)
.\tools\update-offsets.ps1

# Method 2: Via DMA hardware
.\tools\update-offsets.ps1 -Connector pcileech -ConnectorArgs ":device=FPGA"
```

The script calls cs2-dumper from `external/dumper/` and writes results to `data/offsets.json` and `data/client_dll.json`.

> Offsets are version-specific static values. You only need to dump them once after each game update.

---

## Usage

### Startup Flow

1. Ensure `vmm.dll`, `leechcore.dll`, `FTD3XX.dll` are in the same directory as `cs2.exe`
2. Ensure `data/offsets.json` and `data/client_dll.json` match the current game version
3. Connect the FPGA device and run `cs2.exe` on the **secondary machine**
4. The program automatically initializes DMA → searches for cs2.exe → starts rendering

### Hotkeys

| Key | Function |
|-----|----------|
| `F8` | Show / Hide menu |
| `F5` (default, customizable) | Record grenade position |

> All keys are read via DMA from the target machine's keyboard state, but this feature is currently incomplete — keys need to be pressed on the secondary machine's keyboard.

### Config File

Global settings are stored in the program directory in JSON format:

```json
{
    "type": "none",
    "en": "en"
}
```

| Field | Values | Description |
|-------|--------|-------------|
| `en` | `en` / `ch` | UI language (English / Chinese) |
| `type` | `none` / `net` / `net+` / `b` | Peripheral type (None / KmBox Net / Encrypted Net / BPro) |

Feature configs are saved in `saved/configs/`, with support for multiple profiles via the menu.

---

## Developer Guide

### Architecture Overview

The program uses a **multi-threaded + snapshot** architecture:

```
┌─────────────────────┐
│    ConnectionThread  │  Game process lifecycle management (state machine)
├─────────────────────┤
│    DataThread        │  Core data pipeline: matrix → local player → entities → scatter read
├─────────────────────┤
│    SlowUpdateThread  │  Low-frequency: entity list base address, map name
├─────────────────────┤
│    KeysCheckThread   │  Keyboard state polling (DMA reads kernel key state)
├─────────────────────┤
│    WebRadarThread    │  WebSocket broadcast: GameSnapshot → JSON
├─────────────────────┤
│    Main Thread       │  ImGui window + ESP rendering (read-only Snapshot)
└─────────────────────┘
```

**Data flow**: `DataThread` reads game data via DMA and writes to `Cheats::Snapshot` (protected by `shared_mutex`). The render thread and WebRadar thread access the snapshot in read-only mode.

### Key Design Decisions

- **On-demand reading**: `DataThread` dynamically determines which scatter fields to request based on currently enabled features in `MenuConfig`. The entire pipeline sleeps when no features are enabled.
- **Entity caching**: Controller data (names, team, etc.) is not re-read every frame. Instead, it uses tiered update frequencies: `DISCOVERY_INTERVAL` (5 frames) and `CONTROLLER_REFRESH` (50 frames), significantly reducing DMA read count.
- **Scatter batch reading**: All dynamic entity fields (position, health, bones, etc.) are combined into a single scatter batch — one DMA operation.
- **Snapshot mode**: The writer holds `unique_lock` only briefly during data swap; readers use `shared_lock`, so render FPS is not blocked by the data thread.
- **Log ring buffer**: The last 64 log entries are stored in a fixed-size ring buffer. On crash, CrashHandler can dump them directly without filesystem access.

### Code Conventions

- **Naming**: Classes `PascalCase`, functions `PascalCase`, variables `camelCase`, macros/constants `UPPER_SNAKE_CASE`
- **Headers**: Use `#pragma once`
- **Memory reads**: Always through `ProcessMgr` (`ProcessManager` singleton); never call VMMDLL APIs directly
- **Config items**: Add to `MenuConfig.h` (inline globals), UI controls in `GUI.cpp`
- **Logging**: Use `LOG_INFO`, `LOG_ERROR`, etc. — format: `LOG_INFO("ModuleName", "message {}", value)`
- **Thread safety**: Shared data protected via `Cheats::SnapshotMutex`; never read DMA directly from the render thread

### Adding a New Feature

1. **Add config item in `MenuConfig.h`** (e.g. `inline bool ShowNewFeature = false;`)
2. **Add UI control in `GUI.cpp`** (Checkbox / Slider under the appropriate tab)
3. **Add serialization in `ConfigSaver.cpp`** (SaveConfig / LoadConfig)
4. **Add multi-language string in `Language.h`**
5. **If additional data is needed**: Add scatter fields in `DataThread` (`Threads.cpp`) and update the `GameSnapshot` struct
6. **Implement render logic in `Cheats.cpp` or `Render.cpp`**
7. **Test**: Ensure no extra DMA reads occur when the feature is disabled (on-demand reading principle)

### Offset System

Offsets are dynamically loaded from JSON files (`Offsets.cpp` → `Offset::UpdateOffsets()`), not hardcoded. JSON files are generated by [cs2-dumper](https://github.com/a2x/cs2-dumper). To add a new offset:

1. Declare `inline DWORD NewOffset;` in `Offsets.h`
2. Add JSON parsing logic in `Offsets.cpp` `UpdateOffsets()`
3. Use `Offset::NewOffset` in `Entity.cpp` or other modules

---

## Known Issues

- **Offset expiry**: Offsets may become invalid after each CS2 update — use `tools/update-offsets.ps1` to re-obtain
- **Windows keyboard state**: Different Win11 versions have different `gafAsyncKeyState` kernel offsets. The program includes both PDB resolution and hardcoded offset strategies; in rare cases, manual offset table updates may be needed
- **FPGA compatibility**: Only tested with common FPGA DMA devices; other devices may require adjustments to `InitDMA()` parameters
- **Anti-cheat**: This project is for educational and research purposes. Use at your own risk

---

## Credits

- [CS2_DMA_Extrnal](https://github.com/Mzzzj/CS2_DMA_Extrnal) — Initial codebase
- [MemProcFS](https://github.com/ufrisk/MemProcFS) — DMA memory access framework
- [cs2-dumper](https://github.com/a2x/cs2-dumper) — Automated offset dumper
- [cs2_webradar](https://github.com/clauadv/cs2_webradar) — Web Radar frontend
- [Dear ImGui](https://github.com/ocornut/imgui) — GUI framework

## License

This project is licensed under the [MIT License](docs/LICENSE).
