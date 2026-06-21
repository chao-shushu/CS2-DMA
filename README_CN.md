**[English](README.md)** | **中文**

# CS2-DMA

![3feefde8e0a9558fad9e715bdb83c18c](https://github.com/user-attachments/assets/868af1fa-e5d1-4714-a7d9-ca4ba2f1a506)

基于 DMA（Direct Memory Access）硬件的 CS2 外部辅助工具，使用 C++ 开发，通过 FPGA 设备读取游戏内存，在独立副机上渲染 ESP、雷达、投掷物助手等功能。本项目不包含且以后也不会包含kmbox等实现的自瞄相关功能,菜就多练

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Visual Studio](https://img.shields.io/badge/IDE-Visual%20Studio%202026-5C2D91?logo=visual-studio\&logoColor=white)
![Platform](https://img.shields.io/badge/platform-Windows%20x64-0078D6)
![License](https://img.shields.io/badge/license-MIT-green)
另：项目可制成卡密、激活形式、定制功能等，诚招代理，批量提卡

> ⭐ 如果你喜欢本项目，请为仓库点亮Star，支持作者持续更新！
> 如果你想要联系我,你可以通过qq:3594296990或者邮箱:<kuchao1012@outlook.com>(不常看)

***

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
- **观众列表** — 检测谁在观战你，显示观众名称、队伍颜色、观战模式（\[Eye]/\[Chase]/\[Roam]）
- **ESP 预览** — 预览窗口支持单色模式（1 个假玩家）与三色模式（Default/Visible/Hidden 三个假玩家），无需进入对局即可预览所有 ESP 元素效果
- **声音 ESP** — 敌方开火时在其位置显示波纹动画
- **玩家状态标志** — 敌方头顶显示状态标签：致盲 / 开镜 / 拆弹 / 拆弹器 / 金钱
- **武器图标 ESP** — 使用 weapons.ttf 字形字体渲染武器图标（可选隐藏匕首图标）
- **武器弹药 ESP** — 显示弹药量（XX/YY），低弹药时显示警告颜色
- **条形数值标签** — 在血条/护甲条旁显示数字数值
- **世界投掷物计时器** — 世界空间倒计时，支持烟雾弹/燃烧弹/诱饵弹独立开关
- **地上武器 ESP** — 显示地面掉落武器的名称和图标，支持按武器类别过滤
- **炸弹计时器覆盖层** — 可拖动的屏幕 C4 倒计时窗口，含拆弹进度条和拆弹器检测

<img width="1905" height="721" alt="image" src="https://github.com/user-attachments/assets/1481cf2b-075c-40dd-a895-a9cde3d9bf9d" />
### 准星覆盖层（融合器 Tab）
- 4 种准星样式：十字 / 圆点 / 圆圈 / 十字+圆点
- 可调臂长、粗细、间隙、颜色
- 瞄准敌人变色：准星对准敌人时自动变为自定义颜色
- 准星安全区集成

### 炸弹 ESP

- 炸弹已安装 / 被携带 / 掉落 / 正在拆除状态显示

<img width="969" height="554" alt="image" src="https://github.com/user-attachments/assets/e9cd275e-eed7-4f2f-b860-6d9041a4253c" />

### 投掷物 ESP

- 实时显示飞行中的闪光弹、烟雾弹、HE、燃烧弹、诱饵弹

<img width="1874" height="701" alt="image" src="https://github.com/user-attachments/assets/ca3c1dbe-4044-4bd9-b57f-d39eddd66ca5" />

### Web Radar（网页雷达）

- 内嵌 WebSocket 服务器（默认端口 22006，可在菜单中修改）
- 前端资源已嵌入 cs2.exe，单文件部署，无需额外文件
- 三传输层自动降级：WebSocket → SSE → HTTP 轮询
- **局域网访问** — 菜单中打开 Web Radar 后，局域网内任何设备浏览器即可查看实时雷达（菜单显示本机 IP + 一键复制 URL）
- **公网访问（内置 Cloudflare 隧道）** — 菜单中一键启停 cloudflared quick tunnel，自动捕获公网 URL（需先安装 cloudflared：`winget install Cloudflare.cloudflared`）
- **其他公网方案** — 见下方 [公网访问配置](#公网访问配置) 章节（ngrok / frp / 端口转发）
- **DMA 健康状态机** — 三态监控（Healthy / Degraded / Failed），跟踪 DMA 读取成功/失败连续次数，自动恢复
- **VT/IOMMU 拒绝检测** — 连续 3 次 Full refresh 失败后判定疑似 VT/IOMMU 拦截并警告用户
- **RTT 监视器** — /api/ping 端点 + 前端延迟指示器，三态状态（连接中/在线/错误）
- **Page Update 自动刷新** — 资源更新时推送 pageUpdate 事件，客户端自动刷新浏览器
- **Origin 白名单** — CORS 控制，限制 /api/\* 端点允许的来源域名
- **密码鉴权** — 可选的 WebSocket URL query 参数（?password=xxx）访问保护
- **每地图雷达校准** — 按地图调整旋转、缩放、X/Y 偏移，自动保存
- **运行时统计面板** — 实时统计：推送频率、客户端数、发送/丢弃帧数、合并帧数、payload 字节数、每秒字节数
- **队伍面板** — 前端 T/CT 玩家卡片，含角色模型图片、武器图标、弹药、道具栏
- **观战者面板** — 前端观战者列表，带队伍颜色边框
- **提示指示器** — 前端状态提示：安放 / 拆弹 / 独苗
- **炸弹逃生箭头** — 炸弹安放时前端显示方向箭头，指示逃生路线

<img width="773" height="457" alt="image" src="https://github.com/user-attachments/assets/5fb4025a-d06b-4f2e-b81f-ba9b55281252" />

### Grenade Helper（投掷物助手）

- 按地图加载预设投掷点位（JSON 格式）
- 实时方向引导箭头 + 距离提示
- 支持录制/编辑/删除自定义点位

### 快捷键（Hotkeys）

- 16 种动作自定义按键绑定（方框/骨骼/血量/武器/名称/距离/视线/连线/炸弹/投掷物/观众/队伍过滤/雷达/安全区/准星/重新获取数据）
- 双源按键检测：DMA（宿主机）+ `GetAsyncKeyState`（本机）
- 配置持久化保存/加载

### DMA 低延迟优化

- **Scatter 批量读取** — 所有实体数据合并为单次 DMA 操作，避免逐个读取带来的 PCIe 往返延迟
- **按需读取** — 仅读取当前启用功能所需的字段，未开启任何功能时整个数据管线休眠，零无效传输
- **实体缓存分层** — 高频数据（位置、血量）每帧读取，低频数据（名称、队伍）按 5/50 帧间隔更新，减少 80%+ 冗余读取
- **快照零拷贝** — `DataThread` 写入 `GameSnapshot` 时仅短暂加锁交换指针，渲染线程无阻塞读取，数据延迟 < 1 帧, 和拖框说拜拜
- **骨骼可靠性检查** — 校验骨骼数据合理性，缓存上次有效骨骼 150ms，抑制骨骼闪烁
- **快照插值** — 五次缓动平滑 + 速度外推，消除 DMA 帧间玩家位置抖动
- **CameraWorker 线程** — 独立 500Hz 后台线程读取视图矩阵，与渲染循环解耦，消除鼠标转动卡顿

### 性能监控面板

- 屏幕覆盖层：FPS（颜色编码）、帧时间、实体数、投掷物数、观众数
- 在 Settings 标签页中开关

### 其他

- **配置系统** — 创建 / 保存 / 加载 / 删除多套配置，启动自动加载 `_autosave.config`
- **启动自动检查更新** — 启动时对比 GitHub Releases 版本，有新版本时提示跳转下载页面
- **DMA 偏移值集成更新** — 检测到游戏更新时自动运行 cs2-dumper DMA 模式提取偏移值，替代在线比较方案
- **游戏版本自动检查** — 通过 Steam API 获取 CS2 最新新闻时间戳，与本地偏移日期对比，检测偏移是否过期
- **系统代理自动检测** — 自动检测并使用系统代理（手动代理/PAC/WPAD）连接 GitHub 和 Steam API，方便国内用户
- **多显示器支持** — 枚举所有显示器，选择目标显示器进行覆盖层渲染，窗口自动定位
- **渲染分辨率设置** — 分辨率预设（4:3、16:9、16:10）或自动检测，DPI 感知渲染
- **调试日志开关** — 运行时 TRACE/DEBUG 级别日志开关，关闭时零开销
- **调试统计 Overlay** — 详细 7 阶段计时（矩阵/本地玩家/实体/Scatter/武器/炸弹/投掷物）+ WebRadar 统计覆盖层
- **帮助按钮** — Settings 菜单中快速跳转项目 GitHub 页面
- **多语言** — 自动检测系统语言（`GetUserDefaultUILanguage()`），中文系统显示中文，其余显示英文；支持手动切换
- **日志系统** — 分级日志（TRACE → FATAL），环形缓冲区供崩溃诊断
- **崩溃处理** — SEH + <code>std::terminate</code> 捕获，自动生成 <code>.log</code> + <code>.dmp</code>，含最近日志、功能状态、系统信息
- **极致稳定** — 由于dma传输的特殊性,不可避免会产生脏数据,本项目着重优化相关并对数据可靠性做校验,不闪框不漏人
- **分级 DMA 刷新** — 渐进式恢复：Probe（100 次失败）→ Repair（300 次）→ Full（500 次），含地址重新初始化和缓存重置
- **自适应分片发现** — 根据实体数量动态将实体列表扫描拆分为 1/2/4/10 分片，优化吞吐量
- **数据可靠性容忍** — 多种宽限期（核心数据陈旧保持、零 pawn 宽限、层级缺失保持、控制器缺失保持、死亡确认），容忍 DMA 抖动不闪框
- **关于加密解密** — 支持 CR3（DTB）修复且在检测到加密会自动开启
- **VTD** — 不建议开启,虽然已经对dma读取频率做了优化但是仍有可能被检测
- **平台可用性** — 主流对战平台均可使用,但请不要用于真人对战

***

## 快速上手

### 下载

前往 [Releases](https://github.com/chao-shushu/CS2-DMA/releases) 页面下载最新版本的 `CS2-DMA-Release.zip`。

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
3. **在副机上运行** **`cs2.exe`** — 程序会自动完成以下流程：
   - 初始化 DMA 连接
   - 搜索 `cs2.exe` 进程
   - 检测到游戏后自动开始渲染 ESP
4. **按** **`F8`** **打开菜单** — 在菜单中开启/关闭各项功能

### 菜单功能说明

| Tab          | 功能                                                                  |
| ------------ | ------------------------------------------------------------------- |
| **Visuals**  | 方框、骨骼、血条、护甲条、武器、距离、名称、视线、连线、观众列表等 ESP 功能                            |
| **Radar**    | Web Radar 开关、端口、推送频率、局域网 URL 显示与复制、Cloudflare 隧道一键启停、雷达校准（旋转/缩放/偏移） |
| **Grenade**  | 投掷物助手开关、录制点位、编辑/删除                                                  |
| **Fusion**   | 准星覆盖层（4 种样式 + 瞄准敌人变色）、准星安全区                                         |
| **Hotkeys**  | 16 种动作自定义按键绑定（ESP 开关、功能开关、重新获取数据）                                   |
| **Settings** | 帧率限制、VSync、语言切换、队友过滤、显示器选择、分辨率、性能监控、调试日志                            |
| **Config**   | 配置文件的创建、保存、加载、删除                                                    |

### 偏移量过期怎么办？

CS2 每次更新后游戏偏移量可能失效，表现为 ESP 不显示或数据异常。解决方法:

1. 程序可自动检测游戏更新并通过 DMA 模式运行 cs2-dumper 提取新偏移值
2. 也可从 [cs2-dumper](https://github.com/a2x/cs2-dumper) 获取最新 <code>offsets.json</code> 和 <code>client\_dll.json</code>，替换 <code>data/</code> 目录下的文件
3. 或使用 `tools/update-offsets.bat` 手动更新（支持本地和 DMA 模式）
4. 重启程序即可

***

## 反馈 Bug

发现问题？请通过 [GitHub Issues](https://github.com/chao-shushu/CS2-DMA/issues) 提交 Bug 报告。

### 提交 Issue 时请包含以下信息

1. **问题描述** — 简明描述你遇到的问题
2. **复现步骤** — 如何触发这个 Bug
3. **日志文件** — 程序运行目录 <code>logs/</code> 下的最新 <code>.log</code> 文件
4. **崩溃转储**（如果程序崩溃）— <code>crash\_*.log</code>* *和* *<code>crash\_*.dmp</code> 文件，位于 <code>logs/</code> 目录
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

***

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
│   └── update-offsets.bat      # 偏移量更新工具（支持本地/DMA 模式）
├── external/                   # 外部工具
│   ├── dumper/                 # cs2-dumper（Rust，用于获取偏移量）
│   └── webradar/               # cs2_webradar 前端（React）
├── docs/                       # 文档
│   ├── edit-history.md         # 开发变更记录
│   └── LICENSE                 # MIT 许可证
└── dma.slnx                    # Visual Studio 解决方案
```

***

## 构建指南

### 环境要求

| 依赖            | 版本                   |
| ------------- | -------------------- |
| Visual Studio | 2026 Community 或更高版本 |
| C++ 标准        | C++17                |
| 平台            | x64                  |
| Windows SDK   | 10.0+                |

### 运行时依赖

以下 DLL 需与编译产物 `cs2.exe` 放在同一目录：

| 文件              | 说明                      |
| --------------- | ----------------------- |
| `vmm.dll`       | MemProcFS 核心库           |
| `leechcore.dll` | LeechCore 设备通信层         |
| `FTD3XX.dll`    | FTDI USB3 驱动（FPGA 设备需要） |

> 这些 DLL 来自 [MemProcFS](https://github.com/ufrisk/MemProcFS) 发布包，项目仓库中已包含。

### 编译步骤

```powershell
# 1. 克隆仓库
git clone https://github.com/chao-shushu/CS2-DMA.git
cd CS2-DMA

# 2. 使用 Visual Studio 打开 dma.slnx，或命令行编译：
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" `
    "dma.slnx" /p:Configuration=Release /p:Platform=x64 /t:Rebuild /m

# 编译产物：cs2.exe
```

### 偏移量更新

游戏更新后偏移量会失效，需要重新获取：

```bat
# 运行 tools/update-offsets.bat，从菜单选择模式：
#   1. Native Mode — 在运行 CS2 的机器上执行（需要管理员权限）
#   2. DMA Mode — 通过 FPGA 硬件读取内存
```

工具会自动调用 `external/dumper/` 中的 cs2-dumper，将结果写入 `data/offsets.json` 和 `data/client_dll.json`。

> 偏移量是版本相关的静态值，游戏更新后获取一次即可，无需每次启动都运行。

***

## 使用说明

### 启动流程

1. 确保 <code>vmm.dll</code>、<code>leechcore.dll</code>、<code>FTD3XX.dll</code> 与 <code>cs2.exe</code> 在同一目录
2. 确保 <code>data/offsets.json</code> 和 <code>data/client\_dll.json</code> 对应当前游戏版本
3. 连接 FPGA 设备，在**副机**上运行 `cs2.exe`
4. 程序自动初始化 DMA → 搜索 cs2.exe 进程 → 开始渲染

### 热键

| 按键            | 功能        |
| ------------- | --------- |
| `F8`          | 显示 / 隐藏菜单 |
| `F5`（默认，可自定义） | 录制投掷物点位   |

> 按键检测支持双源：DMA 读取宿主机键盘状态 + `GetAsyncKeyState` 读取本机键盘。可在快捷键 Tab 中自定义按键绑定。

### 配置文件

全局设置存储在程序运行目录，使用 JSON 格式：

```json
{
    "en": ""
}
```

| 字段   | 可选值                | 说明                                                                    |
| ---- | ------------------ | --------------------------------------------------------------------- |
| `en` | `""` / `en` / `ch` | 界面语言（自动检测 / English / 中文）；默认空字符串时通过 `GetUserDefaultUILanguage()` 自动检测 |

功能配置保存在 `saved/configs/` 目录，支持通过菜单创建多套配置。

***

## 公网访问配置

Web Radar 默认监听 `0.0.0.0:22006`，局域网内设备可直接访问。如需从公网访问，可使用以下方案。

### 方案一：内置 Cloudflare 隧道（推荐，免费，一键启停）

程序内置 cloudflared quick tunnel 管理，无需手动命令行操作。

1. 安装 cloudflared（仅需一次）：

```bat
winget install Cloudflare.cloudflared
```

1. 在 cs2.exe 菜单 → Radar Tab → "公网访问 (Cloudflare 隧道)" → 勾选"启用隧道"
2. 程序自动启动 cloudflared 并捕获公网 URL，显示在菜单中（一键复制）
3. 在任意设备的浏览器打开该 URL 即可访问雷达
4. 取消勾选即可停止隧道

> 程序退出时 cloudflared 进程会自动终止（Job Object 管理），不会残留。
> 临时域名每次启动都会变化。如需固定域名，参考 [Cloudflare Tunnel 官方文档](https://developers.cloudflare.com/cloudflare-one/connections/connect-networks/) 绑定自有域名。

### 方案二：ngrok（免费额度，简单）

1. 注册 [ngrok](https://ngrok.com/) 账号并下载 `ngrok.exe`
2. 配置 authtoken：

```bat
ngrok config add-authtoken YOUR_TOKEN
```

1. 暴露端口：

```bat
ngrok http 22006
```

1. 终端会显示 `https://xxxx.ngrok-free.app` 公网地址

### 方案三：frp（自建服务器，需公网 IP 服务器）

1. 在公网服务器部署 [frps](https://github.com/fatedier/frp)（服务端）
2. 在运行 cs2.exe 的机器部署 frpc（客户端），配置 `frpc.toml`：

```toml
serverAddr = "你的服务器公网IP"
serverPort = 7000

[[proxies]]
name = "webradar"
type = "tcp"
localIP = "127.0.0.1"
localPort = 22006
remotePort = 22006
```

1. 启动 frpc，访问 `http://服务器公网IP:22006`

### 方案四：路由器端口转发（需公网 IP）

1. 登录路由器管理页面，找到"端口转发 / 虚拟服务器"
2. 添加规则：外部端口 `22006` → 内部 IP（运行 cs2.exe 的机器局域网 IP）→ 内部端口 `22006` → 协议 TCP
3. 访问 `http://你的公网IP:22006`

> 注意：此方案要求你的宽带拥有公网 IP，部分运营商需申请公网 IP 或使用 DDNS。

### 安全提示

- 公网暴露后任何人知道 URL 都可访问你的雷达，建议在 cs2.exe 菜单的 **Origin 白名单** 中限制允许的来源域名
- 临时使用完毕后及时关闭隧道/转发，避免长期暴露
- cloudflared / ngrok 的免费临时域名每次启动都会变化

***

## 开发指南

### 架构概览

程序采用**多线程 + 快照**架构：

```
┌─────────────────────┐
│    ConnectionThread │  游戏进程生命周期管理（状态机）
├─────────────────────┤
│    DataThread       │  核心数据管线：矩阵 → 本地玩家 → 实体 → Scatter 读取
├─────────────────────┤
│    SlowUpdateThread │  低频更新：实体列表基址、地图名
├─────────────────────┤
│    KeysCheckThread  │  键盘状态轮询（DMA + 本机双源按键检测）
├─────────────────────┤
│    WebRadarThread   │  WebSocket 广播 GameSnapshot → JSON
├─────────────────────┤
│    主线程 (Render)   │  ImGui 窗口 + ESP 渲染（只读 Snapshot）
└─────────────────────┘
```

**数据流** — `DataThread` 通过 DMA 读取游戏数据，写入 `Cheats::Snapshot`（`shared_mutex` 保护），渲染线程和 WebRadar 线程以只读方式访问快照。

### 关键设计决策

- **按需读取** — `DataThread` 根据 `MenuConfig` 中当前启用的功能，动态决定 Scatter 请求的字段集合。未启用任何功能时整个管线休眠。
- **实体缓存** — 控制器数据（名称、队伍等）不是每帧都重新读取，而是以 `DISCOVERY_INTERVAL`（5帧）和 `CONTROLLER_REFRESH`（50帧）两个频率分层更新，大幅减少 DMA 读取次数。
- **Scatter 批量读取** — 所有实体的动态字段（位置、血量、骨骼等）合并到一个 Scatter 批次中，一次 DMA 操作完成。
- **Snapshot 快照模式** — 写线程持有 `unique_lock` 仅在交换数据时短暂加锁，读线程用 `shared_lock`，渲染帧率不受数据线程阻塞。
- **日志环形缓冲区** — 最近 64 条日志保存在固定大小的环形缓冲区中，崩溃时 CrashHandler 可直接 dump，无需访问文件系统。

### 代码规范

- **命名** — 类名 <code>PascalCase</code>，函数名 <code>PascalCase</code>，变量名 <code>camelCase</code>，宏/常量 <code>UPPER\_SNAKE\_CASE</code>
- **头文件** — 使用 `#pragma once`
- **内存读取** — 统一通过 <code>ProcessMgr</code>（<code>ProcessManager</code> 单例）进行，禁止直接调用 VMMDLL API
- **配置项** — 新增功能的配置项添加到 <code>MenuConfig.h</code>（inline 全局变量），UI 控件添加到 <code>GUI.cpp</code>
- **日志** — 使用 `LOG_INFO`、`LOG_ERROR` 等宏，格式为 `LOG_INFO("ModuleName", "message {}", value)`
- **线程安全** — 共享数据通过 <code>Cheats::SnapshotMutex</code> 保护，不要在渲染线程中直接读取 DMA

### 添加新功能的流程

1. **在** **`MenuConfig.h`** **添加配置项**（如 `inline bool ShowNewFeature = false;`）
2. **在** **`GUI.cpp`** **添加菜单控件**（对应 Tab 下添加 Checkbox / Slider 等）
3. **在** **`ConfigSaver.cpp`** **添加序列化**（SaveConfig / LoadConfig 中追加字段）
4. **在** **`Language.h`** **添加多语言字符串**
5. **如需额外数据**：在 `DataThread`（`Threads.cpp`）的 Scatter 请求中添加字段，更新 `GameSnapshot` 结构体
6. **在** **`Cheats.cpp`** **或** **`Render.cpp`** **中实现渲染逻辑**
7. **测试**：确保关闭该功能时不产生额外 DMA 读取（按需读取原则）

### 偏移量体系

偏移量从 JSON 文件动态加载（`Offsets.cpp` → `Offset::UpdateOffsets()`），而非硬编码。JSON 由 [cs2-dumper](https://github.com/a2x/cs2-dumper) 生成。添加新偏移量时：

1. 在 `Offsets.h` 中声明 `inline DWORD NewOffset;`
2. 在 `Offsets.cpp` 的 `UpdateOffsets()` 中添加 JSON 解析逻辑
3. 在 `Entity.cpp` 或其他模块中使用 `Offset::NewOffset`

***

## 已知问题与注意事项

- **Windows 键盘状态**：Win11 不同版本的 `gafAsyncKeyState` 内核偏移不同，程序内置特征码扫描、PDB 解析、硬编码偏移三套策略，极端情况下可能需要手动更新偏移表
- **FPGA 兼容性**：仅测试过常见 FPGA DMA(75t加固件) 设备，其他设备可能需要调整 `InitDMA()` 的参数
- **反作弊**：虽然是只读类型的dma不容易被检测,但请注意本项目为学习和研究目的,不为盈利!使用者需自行承担风险!!!

***

## 致谢

- [CS2\_DMA\_Extrnal](https://github.com/Mzzzj/CS2_DMA_Extrnal) — 初始代码基础与灵感来源
- [MemProcFS](https://github.com/ufrisk/MemProcFS) — DMA 内存访问框架
- [cs2-dumper](https://github.com/a2x/cs2-dumper) — 偏移量自动化工具
- [cs2\_webradar](https://github.com/clauadv/cs2_webradar) — Web Radar 前端
- [Dear ImGui](https://github.com/ocornut/imgui) — GUI 框架

## 许可证

本项目基于 [MIT License](docs/LICENSE) 发布。
