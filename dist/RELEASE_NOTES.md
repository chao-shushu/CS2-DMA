## 功能

- **ESP / Visuals** — 方框（普通/动态/圆角）、骨骼、血量条、护甲条、武器、距离、玩家名称、视线射线、连线、头部圆点、安全区域
- **炸弹 ESP** — 安装/携带/掉落/拆除状态 + 倒计时
- **投掷物 ESP** — 飞行中闪光/烟雾/HE/燃烧弹/诱饵弹 + 效果范围圈
- **Web Radar** — 内嵌 WebSocket 服务器，局域网浏览器查看实时雷达
- **Grenade Helper** — 按地图加载投掷点位，实时方向引导，支持录制自定义点位
- **配置系统** — 多配置创建/保存/加载/删除，启动自动加载
- **多语言** — 中文 / English
- **日志 & 崩溃处理** — 分级日志 + SEH/MiniDump 自动生成崩溃报告

## 运行要求

- Windows 10/11 x64
- FPGA DMA 硬件设备（如 Squirrel / 75T 等）
- 偏移量需对应当前 CS2 版本（附带的偏移量截止 2026-04-12）

## 使用方法

1. 解压 zip 到任意目录
2. 确保 FPGA 设备已连接
3. 运行 `cs2.exe`
4. 程序自动连接 DMA → 搜索游戏进程 → 开始渲染
5. `F8` 打开/关闭菜单

## 压缩包内容

```
CS2-DMA/
├── cs2.exe              # 主程序
├── vmm.dll              # MemProcFS 核心库
├── leechcore.dll        # LeechCore 设备通信
├── FTD3XX.dll           # FTDI USB3 驱动
├── data/
│   ├── offsets.json     # 游戏偏移量
│   ├── client_dll.json  # client.dll 偏移量
│   └── grenade-helper/  # 投掷物助手数据
├── saved/configs/       # 配置存储目录
└── logs/                # 日志目录
```

## 注意事项

- 游戏更新后偏移量可能失效，需从源码仓库使用 `tools/update-offsets.ps1` 重新获取
- 本项目仅供学习研究，使用者需自行承担风险
