#pragma once

#include "Game.h"

#include "Bone.h"

#include <map>
#include <vector>



namespace MenuConfig

{

	inline int MaxFrameRate = 0;
	inline bool VSync = false;

	// ======== Render Resolution ========
	inline int RenderWidth = 0;   // 0 = auto (monitor resolution)
	inline int RenderHeight = 0;  // 0 = auto (monitor resolution)

	// ======== Monitor Selection ========
	inline int MonitorIndex = 0;  // 0 = primary monitor, 1+ = secondary

	struct MonitorDesc {
		int index;
		int x, y, width, height;
		std::string name; // e.g. "Monitor 1 (1920x1080)"
	};
	inline std::vector<MonitorDesc> MonitorList;



	inline int SelectedLanguage = 0;



	inline std::string path = "saved/configs";



	inline bool ShowBoneESP = false;

	inline bool ShowBoxESP = true;

	inline bool ShowHealthBar = true;

	inline bool ShowWeaponESP = true;

	inline bool ShowDistance = false;

	inline bool ShowEyeRay = false;

	inline bool ShowPlayerName = true;



	// 0: normal 1: dynamic 2: corner

	inline int  BoxType = 0;

	inline int  HealthBarType = 0;



	inline ImColor BoneColor = ImColor(255, 255, 255, 255);

	inline ImColor BoxColor = ImColor(255, 255, 255, 255);

	inline ImColor EyeRayColor = ImColor(255, 0, 0, 255);



	inline bool ShowMenu = true;

	// ======== Web Radar ========
	inline bool ShowWebRadar = false;
	inline int  WebRadarPort = 22006;
	inline int  WebRadarInterval = 100; // ms between broadcasts

	inline bool TeamCheck = true;



	inline bool ShowLineToEnemy = false;
	inline ImColor LineToEnemyColor = ImColor(255, 255, 255, 220);

	// ======== Box Customization ========
	inline float BoxThickness = 1.3f;
	inline float BoxRounding = 0.f;
	inline bool  BoxFilled = false;
	inline float BoxFillAlpha = 0.15f;
	// BoxType 2 = Corner Box
	inline float CornerLength = 0.25f; // fraction of edge length

	// ======== Bone Customization ========
	inline float BoneThickness = 1.3f;

	// ======== Head Dot ========
	inline bool  ShowHeadDot = false;
	inline ImColor HeadDotColor = ImColor(255, 0, 0, 255);
	inline float HeadDotSize = 3.f;

	// ======== Armor Bar ========
	inline bool  ShowArmorBar = false;
	// 0: Vertical(right side)  1: Horizontal(top)
	inline int   ArmorBarType = 0;
	inline ImColor ArmorBarColor = ImColor(0, 120, 255, 220);
	inline float ArmorBarWidth = 3.f;

	// ======== Health Bar Width ========
	inline float HealthBarWidth = 4.f;

	// ======== Eye Ray Customization ========
	inline float EyeRayLength = 50.f;
	inline float EyeRayThickness = 1.3f;

	// ======== Snapline Customization ========
	inline float LineToEnemyThickness = 1.2f;
	// 0: Top  1: Center  2: Bottom
	inline int   LineToEnemyOrigin = 0;

	// ======== Bomb ESP ========
	inline bool  ShowBombESP = true;
	inline ImColor BombPlantedColor = ImColor(255, 50, 50, 255);
	inline ImColor BombCarrierColor = ImColor(255, 200, 0, 255);
	inline ImColor BombDroppedColor = ImColor(255, 150, 0, 255);
	inline ImColor BombDefusingColor = ImColor(0, 180, 255, 255);

	// ======== Grenade Projectile ESP ========
	inline bool  ShowProjectileESP = true;
	inline bool  ShowProjectileRange = true;
	inline float ProjectileRangeAlpha = 0.12f;

	// ======== Debug Log ========
	inline bool  DebugLog = false;

	// ======== Safe Zone (Crosshair Cutout) ========
	inline bool  SafeZoneEnabled = false;
	inline float SafeZoneRadius = 100.f;
	inline int   SafeZoneShape = 0; // 0: Circle, 1: Square

	// ======== Crosshair Overlay ========
	inline bool    CrosshairEnabled = false;
	inline float   CrosshairSize = 6.f;       // arm length in px
	inline float   CrosshairThickness = 1.5f;
	inline float   CrosshairGap = 3.f;        // center gap in px
	inline int     CrosshairStyle = 0;        // 0: Cross  1: Dot  2: Circle  3: Cross+Dot
	inline ImColor CrosshairColor = ImColor(0, 255, 0, 255);
	inline bool    CrosshairOnEnemyColor = true;
	inline ImColor CrosshairEnemyColor = ImColor(255, 0, 0, 255);

	// ======== Menu Hotkey ========
	inline int   MenuHotKey = VK_F8;
	inline char  MenuHotKeyName[32] = "F8";
	inline bool  IsListeningForMenuKey = false;

	// ======== Spectator List ========
	inline bool  ShowSpectatorList = false;

	// ======== Performance Monitor ========
	inline bool  ShowPerfMonitor = false;

	// ======== Text Customization ========
	inline ImColor NameColor = ImColor(255, 255, 255, 255);
	inline float NameFontSize = 14.f;
	inline ImColor WeaponColor = ImColor(200, 200, 200, 255);
	inline float WeaponFontSize = 14.f;
	inline ImColor DistanceColor = ImColor(255, 255, 255, 255);
	inline float DistanceFontSize = 14.f;

}
