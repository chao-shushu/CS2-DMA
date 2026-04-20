const translations = {
  en: {
    // Settings panel
    settings: "Settings",
    reset: "Reset",

    // Sections
    sec_radar: "Radar",
    sec_players: "Players",
    sec_display: "Display",
    sec_system: "System",

    // Radar
    zoom: "Zoom",
    dotSize: "Dot Size",
    bombSize: "Bomb Size",

    // Players
    teamNames: "Team Names",
    enemyNames: "Enemy Names",
    showHealth: "Show Health",
    showWeapon: "Show Weapon",
    infoTextSize: "Info Text Size",
    showDeadPlayers: "Show Dead",
    deadOpacity: "Dead Opacity",

    // Display
    viewCones: "View Cones",
    coneSize: "Cone Size",
    playerCards: "Player Cards",
    showBombTimer: "Bomb Timer",
    showLatency: "Show Latency",
    bgOpacity: "BG Opacity",
    smoothing: "Smooth Move",

    // System
    language: "Language",
    lang_cn: "中文",
    lang_en: "English",

    // Status
    connecting: "Connecting to DMA data source...",
    connected: "Connected, waiting for game data...",
  },
  cn: {
    // Settings panel
    settings: "设置",
    reset: "重置",

    // Sections
    sec_radar: "雷达",
    sec_players: "玩家",
    sec_display: "显示",
    sec_system: "系统",

    // Radar
    zoom: "缩放",
    dotSize: "圆点大小",
    bombSize: "炸弹大小",

    // Players
    teamNames: "队友名称",
    enemyNames: "敌人名称",
    showHealth: "显示血量",
    showWeapon: "显示武器",
    infoTextSize: "信息文字大小",
    showDeadPlayers: "显示死亡玩家",
    deadOpacity: "死亡透明度",

    // Display
    viewCones: "视角锥",
    coneSize: "锥体大小",
    playerCards: "玩家卡片",
    showBombTimer: "炸弹计时器",
    showLatency: "显示延迟",
    bgOpacity: "背景透明度",
    smoothing: "移动平滑",

    // System
    language: "语言",
    lang_cn: "中文",
    lang_en: "English",

    // Status
    connecting: "正在连接 DMA 数据源...",
    connected: "已连接，等待游戏数据...",
  },
};

export const getT = (lang) => {
  const dict = translations[lang] || translations.cn;
  return (key) => dict[key] || key;
};

export default translations;
