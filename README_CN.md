**[English](README.md)** | **中文**

# CS2-DMA
![3feefde8e0a9558fad9e715bdb83c18c](https://github.com/user-attachments/assets/868af1fa-e5d1-4714-a7d9-ca4ba2f1a506)

基于 DMA（Direct Memory Access）硬件的 CS2 外部辅助工具，使用 C++ 开发，通过 FPGA 设备读取游戏内存，在独立副机上渲染 ESP、雷达、投掷物助手等功能。本项目不包含且以后也不会包含kmbox等实现的自瞄相关功能,菜就多练

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Visual Studio](https://img.shields.io/badge/IDE-Visual%20Studio%202026-5C2D91?logo=visual-studio&logoColor=white)
![Platform](https://img.shields.io/badge/platform-Windows%20x64-0078D6)
![License](https://img.shields.io/badge/license-MIT-green)

> ⭐ 如果你喜欢本项目，请为仓库点亮Star，支持作者持续更新！

---

## 功能特性

### Visuals / ESP
- **方框 ESP** — 普通 / 动态 / 圆角三种样式，支持填充、圆角、粗细自定义
- **骨骼 ESP** — 完整骨骼连线渲染
- **血量条 / 护甲条** — 水平或垂直样式，带受伤回溯动画
- **武器 ESP** — 显示当前持有武器名称
- **距离 ESP** — 显示与敌方的距离
- **玩家名称** — 显示玩家昵称
- **视线射线** — 显示敌方朝向
- **连线** — 从屏幕边缘到敌方的引导线
- **头部圆点** — 头部位置标记
- **安全区域** — 准心附近 ESP 裁切，减少视觉干扰
<img width="827" height="524" alt="image" src="https://github.com/user-attachments/assets/6589c93a-007a-467f-966b-6b6126f92d60" />

### 炸弹 ESP
- 炸弹已安装 / 被携带 / 掉落 / 正在拆除状态显示
- 爆炸倒计时与拆除倒计时
<img width="837" height="517" alt="image" src="https://github.com/user-attachments/assets/8f7d1785-b21e-41a5-80e2-ebd7e122f4aa" />

### 投掷物 ESP
- 实时显示飞行中的闪光弹、烟雾弹、HE、燃烧弹、诱饵弹
- 爆炸/效果范围圈渲染
<img width="838" height="523" alt="image" src="https://github.com/user-attachments/assets/79e82266-5a15-4029-ab8e-b7b8fa647302" />

### Web Radar（网页雷达）
- 内嵌 WebSocket 服务器
- 使用 [cs2_webradar](https://github.com/clauadv/cs2_webradar) 前端
- 局域网内任何设备浏览器即可查看实时雷达
<img width="840" height="517" alt="image" src="https://github.com/user-attachments/assets/6dbaf41a-4dfd-41f8-85e4-8fa2d9b1a52f" />
<img width="773" height="457" alt="image" src="https://github.com/user-attachments/assets/5fb4025a-d06b-4f2e-b81f-ba9b55281252" />

### Grenade Helper（投掷物助手）
- 按地图加载预设投掷点位（JSON 格式）
- 实时方向引导箭头 + 距离提示
- 支持录制/编辑/删除自定义点位
- 支持闪光、烟雾、HE、燃烧弹四种类型
<img width="1615" height="1031" alt="image" src="https://github.com/user-attachments/assets/9fb5df0e-9699-4a5a-8253-ca65b0d2aad5" />

### DMA 低延迟优化
- **Scatter 批量读取** — 所有实体数据合并为单次 DMA 操作，避免逐个读取带来的 PCIe 往返延迟
- **按需读取** — 仅读取当前启用功能所需的字段，未开启任何功能时整个数据管线休眠，零无效传输
- **实体缓存分层** — 高频数据（位置、血量）每帧读取，低频数据（名称、队伍）按 5/50 帧间隔更新，减少 80%+ 冗余读取
- **快照零拷贝** — `DataThread` 写入 `GameSnapshot` 时仅短暂加锁交换指针，渲染线程无阻塞读取，数据延迟 < 1 帧, 和拖框说拜拜

### 其他
- **配置系统** — 创建 / 保存 / 加载 / 删除多套配置，启动自动加载 `_autosave.config`
- **多语言** — 中文 / 英文切换
- **日志系统** — 分级日志（TRACE → FATAL），环形缓冲区供崩溃诊断
- **崩溃处理** — SEH + `std::terminate` 捕获，自动生成 `.log` + `.dmp`，含最近日志、功能状态、系统信息
- **极致稳定** — 由于dma传输的特殊性,不可避免会产生脏数据,本项目着重优化相关并对数据可靠性做校验,不闪框不漏人
- **关于加密解密** — 支持 CR3（DTB）修复且在检测到加密会自动开启,但不是真正的 CR3 解密!如果遇到加密环境,可以联系作者进行技术支持
- **VTD** — 不建议开启,虽然已经对dma读取频率做了优化但是仍有可能被检测
- **平台可用性** — 主流对战平台均可使用,但请不要用于真人对战
---

## 快速上手

### 下载

前往 [Releases](https://github.com/kuchaomc/CS2-DMA/releases) 页面下载最新版本的 `CS2-DMA-Release.zip`。

### 解压后的目录结构

```
CS2-DMA/
├── cs2.exe              # 主程序
├── vmm.dll              # MemProcFS 核心库
├── leechcore.dll        # LeechCore 设备通信
├── FTD3XX.dll           # FTDI USB3 驱动
├── data/
│   ├── offsets.json     # 游戏偏移量
│   ├── client_dll.json  # client.dll 偏移量
│   └── grenade-helper/  # 投掷物助手地图数据
├── saved/configs/       # 配置存储（自动生成）
└── logs/                # 日志目录（自动生成）
```

### 运行步骤

1. **连接 FPGA 设备** — 确保 DMA 硬件已正确连接到副机
2. **在主机上启动 CS2** — 正常打开游戏并进入对局
3. **在副机上运行 `cs2.exe`** — 程序会自动完成以下流程：
   - 初始化 DMA 连接
   - 搜索 `cs2.exe` 进程
   - 检测到游戏后自动开始渲染 ESP
4. **按 `F8` 打开菜单** — 在菜单中开启/关闭各项功能

### 菜单功能说明

| Tab | 功能 |
|-----|------|
| **Visuals** | 方框、骨骼、血条、护甲条、武器、距离、名称、视线、连线等 ESP 功能 |
| **Radar** | Web Radar 开关、端口、推送频率 |
| **Grenade** | 投掷物助手开关、录制点位、编辑/删除 |
| **Settings** | 帧率限制、VSync、语言切换、队友过滤 |
| **Config** | 配置文件的创建、保存、加载、删除 |

### 偏移量过期怎么办？

CS2 每次更新后游戏偏移量可能失效，表现为 ESP 不显示或数据异常。解决方法：

1. 从 [cs2-dumper](https://github.com/a2x/cs2-dumper) 获取最新的 `offsets.json` 和 `client_dll.json`
2. 替换 `data/` 目录下的对应文件
3. 重启程序即可

> 如果你是从源码构建的，也可以使用 `tools/update-offsets.ps1` 脚本自动更新。

---

## 反馈 Bug

发现问题？请通过 [GitHub Issues](https://github.com/kuchaomc/CS2-DMA/issues) 提交 Bug 报告。

### 提交 Issue 时请包含以下信息

1. **问题描述** — 简明描述你遇到的问题
2. **复现步骤** — 如何触发这个 Bug
3. **日志文件** — 程序运行目录 `logs/` 下的最新 `.log` 文件
4. **崩溃转储**（如果程序崩溃）— `logs/` 下的 `crash_*.log` 和 `crash_*.dmp` 文件
5. **环境信息**：
   - Windows 版本（如 Win11 24H2）
   - FPGA 设备型号
   - CS2 是否刚更新过（偏移量是否最新）

### 日志和崩溃文件在哪？

程序运行时会在 `logs/` 目录下自动生成：
- `cs2dma_YYYYMMDD_HHMMSS.log` — 运行日志
- `crash_YYYYMMDD_HHMMSS.log` — 崩溃诊断报告（含最近日志、功能状态、系统信息）
- `crash_YYYYMMDD_HHMMSS.dmp` — MiniDump 转储文件

> 提交 Issue 时附上这些文件能帮助快速定位问题。

---

## 项目结构

```
CS2-DMA/
├── cs2/                        # 主项目源码
│   ├── main.cpp                # 入口：初始化日志、DMA、线程、渲染窗口
│   ├── game/                   # 游戏逻辑层
│   │   ├── Threads.cpp/h       # 线程定义（Connection / Data / SlowUpdate / Keys / WebRadar）
│   │   ├── Entity.cpp/h        # 实体数据结构与读取
│   │   ├── Bone.cpp/h          # 骨骼定义与解析
│   │   ├── Game.cpp/h          # 游戏地址初始化
│   │   ├── Offsets.cpp/h       # 偏移量解析（从 JSON 动态加载）
│   │   ├── GlobalVars.cpp/h    # 全局变量读取
│   │   ├── AppState.h          # 应用状态机枚举
│   │   ├── MenuConfig.h        # 所有菜单配置项（inline 全局变量）
│   │   └── View.h              # 视图矩阵
│   ├── render/                 # 渲染层
│   │   ├── Cheats.cpp/h        # ESP 主渲染入口 + GameSnapshot 定义
│   │   ├── Render.cpp/h        # 渲染工具函数（方框、骨骼、血条等）
│   │   ├── GUI.cpp/h           # ImGui 菜单界面
│   │   ├── GrenadeHelper.cpp/h # 投掷物助手
│   │   └── WebRadar.cpp/h      # WebSocket 服务器 + 雷达数据序列化
│   ├── config/                 # 配置系统
│   │   ├── ConfigSaver.cpp/h   # 配置文件读写
│   │   ├── ConfigMenu.cpp/h    # 配置菜单 UI
│   │   ├── SettingsManager.cpp/h # 全局设置（语言等）
│   │   └── Language.h          # 多语言字符串
│   ├── utils/                  # 工具模块
│   │   ├── Logger.cpp/h        # 日志系统（单例、线程安全、环形缓冲区）
│   │   ├── CrashHandler.cpp/h  # 崩溃处理（MiniDump + 诊断报告）
│   │   ├── ProcessManager.h    # DMA 内存读取封装（VMMDLL）
│   │   └── base64.h            # Base64 工具
│   ├── includes/               # 第三方头文件（vmmdll.h、leechcore.h、rapidjson）
│   ├── SDK/                    # VMMDLL 库文件（Lib/Include）
│   └── OS-ImGui/               # ImGui 渲染框架封装
├── data/                       # 运行时数据
│   ├── offsets.json            # 游戏偏移量
│   ├── client_dll.json         # client.dll 偏移量
│   └── grenade-helper/         # 投掷物助手地图数据（JSON）
├── saved/                      # 用户配置存储目录
├── logs/                       # 日志和崩溃转储
├── tools/                      # 自动化脚本
│   ├── update-offsets.ps1      # 偏移量更新脚本（支持本地/DMA 模式）
│   └── update-offsets.bat      # 批处理入口
├── external/                   # 外部工具
│   ├── dumper/                 # cs2-dumper（Rust，用于获取偏移量）
│   └── webradar/               # cs2_webradar 前端（React）
├── docs/                       # 文档
│   ├── webradar-setup.md       # Web Radar 部署指南
│   ├── edit-history.md         # 开发变更记录
│   └── LICENSE                 # MIT 许可证
└── dma.slnx                    # Visual Studio 解决方案
```

---

## 构建指南

### 环境要求

| 依赖 | 版本 |
|------|------|
| Visual Studio | 2026 Community 或更高版本 |
| C++ 标准 | C++17 |
| 平台 | x64 |
| Windows SDK | 10.0+ |

### 运行时依赖

以下 DLL 需与编译产物 `cs2.exe` 放在同一目录：

| 文件 | 说明 |
|------|------|
| `vmm.dll` | MemProcFS 核心库 |
| `leechcore.dll` | LeechCore 设备通信层 |
| `FTD3XX.dll` | FTDI USB3 驱动（FPGA 设备需要） |

> 这些 DLL 来自 [MemProcFS](https://github.com/ufrisk/MemProcFS) 发布包，项目仓库中已包含。

### 编译步骤

```powershell
# 1. 克隆仓库
git clone <仓库地址>
cd CS2-DMA

# 2. 使用 Visual Studio 打开 dma.slnx，或命令行编译：
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" `
    "dma.slnx" /p:Configuration=Release /p:Platform=x64 /t:Rebuild /m

# 编译产物：cs2.exe
```

### 偏移量更新

游戏更新后偏移量会失效，需要重新获取：

```powershell
# 方式一：在运行 CS2 的机器上（本地模式）
.\tools\update-offsets.ps1

# 方式二：通过 DMA 硬件
.\tools\update-offsets.ps1 -Connector pcileech -ConnectorArgs ":device=FPGA"
```

脚本会自动调用 `external/dumper/` 中的 cs2-dumper，将结果写入 `data/offsets.json` 和 `data/client_dll.json`。

> 偏移量是版本相关的静态值，游戏更新后获取一次即可，无需每次启动都运行。

---

## 使用说明

### 启动流程

1. 确保 `vmm.dll`、`leechcore.dll`、`FTD3XX.dll` 与 `cs2.exe` 在同一目录
2. 确保 `data/offsets.json` 和 `data/client_dll.json` 对应当前游戏版本
3. 连接 FPGA 设备，在**副机**上运行 `cs2.exe`
4. 程序自动初始化 DMA → 搜索 cs2.exe 进程 → 开始渲染

### 热键

| 按键 | 功能 |
|------|------|
| `F8` | 显示 / 隐藏菜单 |
| `F5`（默认，可自定义） | 录制投掷物点位 |

> 所有按键通过 DMA 读取目标机器的键盘状态，但目前本功能不完善,需在副机键盘上操作。

### 配置文件

全局设置存储在程序运行目录，使用 JSON 格式：

```json
{
    "type": "none",
    "en": "ch"
}
```

| 字段 | 可选值 | 说明 |
|------|--------|------|
| `en` | `en` / `ch` | 界面语言（English / 中文） |
| `type` | `none` / `net` / `net+` / `b` | 外设类型（无 / KmBox Net / 加密 Net / BPro） |

功能配置保存在 `saved/configs/` 目录，支持通过菜单创建多套配置。

---

## 开发指南

### 架构概览

程序采用**多线程 + 快照**架构：

```
┌─────────────────────┐
│    ConnectionThread  │  游戏进程生命周期管理（状态机）
├─────────────────────┤
│    DataThread        │  核心数据管线：矩阵 → 本地玩家 → 实体 → Scatter 读取
├─────────────────────┤
│    SlowUpdateThread  │  低频更新：实体列表基址、地图名
├─────────────────────┤
│    KeysCheckThread   │  键盘状态轮询（DMA 读取内核键盘状态）
├─────────────────────┤
│    WebRadarThread    │  WebSocket 广播 GameSnapshot → JSON
├─────────────────────┤
│    主线程 (Render)    │  ImGui 窗口 + ESP 渲染（只读 Snapshot）
└─────────────────────┘
```

**数据流**：`DataThread` 通过 DMA 读取游戏数据，写入 `Cheats::Snapshot`（`shared_mutex` 保护），渲染线程和 WebRadar 线程以只读方式访问快照。

### 关键设计决策

- **按需读取**：`DataThread` 根据 `MenuConfig` 中当前启用的功能，动态决定 Scatter 请求的字段集合。未启用任何功能时整个管线休眠。
- **实体缓存**：控制器数据（名称、队伍等）不是每帧都重新读取，而是以 `DISCOVERY_INTERVAL`（5帧）和 `CONTROLLER_REFRESH`（50帧）两个频率分层更新，大幅减少 DMA 读取次数。
- **Scatter 批量读取**：所有实体的动态字段（位置、血量、骨骼等）合并到一个 Scatter 批次中，一次 DMA 操作完成。
- **Snapshot 快照模式**：写线程持有 `unique_lock` 仅在交换数据时短暂加锁，读线程用 `shared_lock`，渲染帧率不受数据线程阻塞。
- **日志环形缓冲区**：最近 64 条日志保存在固定大小的环形缓冲区中，崩溃时 CrashHandler 可直接 dump，无需访问文件系统。

### 代码规范

- **命名**：类名 `PascalCase`，函数名 `PascalCase`，变量名 `camelCase`，宏/常量 `UPPER_SNAKE_CASE`
- **头文件**：使用 `#pragma once`
- **内存读取**：统一通过 `ProcessMgr`（`ProcessManager` 单例）进行，禁止直接调用 VMMDLL API
- **配置项**：新增功能的配置项添加到 `MenuConfig.h`（inline 全局变量），UI 控件添加到 `GUI.cpp`
- **日志**：使用 `LOG_INFO`、`LOG_ERROR` 等宏，格式为 `LOG_INFO("ModuleName", "message {}", value)`
- **线程安全**：共享数据通过 `Cheats::SnapshotMutex` 保护，不要在渲染线程中直接读取 DMA

### 添加新功能的流程

1. **在 `MenuConfig.h` 添加配置项**（如 `inline bool ShowNewFeature = false;`）
2. **在 `GUI.cpp` 添加菜单控件**（对应 Tab 下添加 Checkbox / Slider 等）
3. **在 `ConfigSaver.cpp` 添加序列化**（SaveConfig / LoadConfig 中追加字段）
4. **在 `Language.h` 添加多语言字符串**
5. **如需额外数据**：在 `DataThread`（`Threads.cpp`）的 Scatter 请求中添加字段，更新 `GameSnapshot` 结构体
6. **在 `Cheats.cpp` 或 `Render.cpp` 中实现渲染逻辑**
7. **测试**：确保关闭该功能时不产生额外 DMA 读取（按需读取原则）

### 偏移量体系

偏移量从 JSON 文件动态加载（`Offsets.cpp` → `Offset::UpdateOffsets()`），而非硬编码。JSON 由 [cs2-dumper](https://github.com/a2x/cs2-dumper) 生成。添加新偏移量时：

1. 在 `Offsets.h` 中声明 `inline DWORD NewOffset;`
2. 在 `Offsets.cpp` 的 `UpdateOffsets()` 中添加 JSON 解析逻辑
3. 在 `Entity.cpp` 或其他模块中使用 `Offset::NewOffset`

---

## 已知问题与注意事项

- **偏移量时效性**：每次 CS2 更新后偏移量可能失效，需使用 `tools/update-offsets.ps1` 重新获取
- **Windows 键盘状态**：Win11 不同版本的 `gafAsyncKeyState` 内核偏移不同，程序内置了 PDB 解析 + 硬编码偏移两套策略，极端情况下可能需要手动更新偏移表
- **FPGA 兼容性**：仅测试过常见 FPGA DMA(75t加固件) 设备，其他设备可能需要调整 `InitDMA()` 的参数
- **反作弊**：虽然是只读类型的dma不容易被检测,但请注意本项目为学习和研究目的,不为盈利!使用者需自行承担风险!!!

---

## 致谢

- [CS2_DMA_Extrnal](https://github.com/Mzzzj/CS2_DMA_Extrnal) — 初始代码基础与灵感来源
- [MemProcFS](https://github.com/ufrisk/MemProcFS) — DMA 内存访问框架
- [cs2-dumper](https://github.com/a2x/cs2-dumper) — 偏移量自动化工具
- [cs2_webradar](https://github.com/clauadv/cs2_webradar) — Web Radar 前端
- [Dear ImGui](https://github.com/ocornut/imgui) — GUI 框架

## 许可证

本项目基于 [MIT License](docs/LICENSE) 发布。
