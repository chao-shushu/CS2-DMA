#pragma once

#include <Windows.h>
#include <string>
#include <cstdint>

namespace Offset
{
	// Version info
	inline std::string GameUpdateDate;
	inline int64_t GameUpdateTimestamp = 0;
	inline int64_t LatestSteamUpdateTimestamp = 0;
	inline DWORD EntityList;
	inline DWORD Matrix ;
	inline DWORD LocalPlayerController;
	inline DWORD LocalPlayerPawn;
	inline DWORD GlobalVars;
	inline DWORD PlantedC4;
	inline DWORD WeaponC4;

	
	inline	DWORD Health;
	inline	DWORD TeamID;
	inline	DWORD IsAlive;
	inline	DWORD PlayerPawn;
	inline	DWORD iszPlayerName;
	inline	DWORD MoneyService;

	
	inline DWORD Pos;
	inline DWORD MaxHealth;
	inline DWORD CurrentHealth;
	inline DWORD Armor;
	inline DWORD PawnArmor;  // C_CSPlayerPawn::m_ArmorValue (authoritative, on pawn)
	inline DWORD GameSceneNode;
	inline DWORD BoneArray;
	inline DWORD MapName;
	inline DWORD angEyeAngles;
	inline DWORD vecLastClipCameraPos;
	inline DWORD iShotsFired;
	inline DWORD flFlashDuration;
	inline DWORD aimPunchAngle;
	inline DWORD iIDEntIndex;
	inline DWORD iTeamNum;
	inline DWORD CameraServices;
	inline DWORD iFovStart;
	inline DWORD fFlags;
	inline DWORD bSpottedByMask;
	inline DWORD ItemServices;
	inline DWORD HasHelmet;
	inline DWORD HasDefuser;
	inline DWORD CompTeammateColor;
	inline DWORD OwnerEntity;
	inline DWORD vecAbsOrigin;
	inline DWORD ModelStateOffset;
	inline DWORD ModelNameOffset;
	// C_PlantedC4
	inline DWORD BombTicking;
	inline DWORD C4Blow;
	inline DWORD BombDefused;
	inline DWORD BeingDefused;
	inline DWORD DefuseCountDown;
	// Weapon list
	inline DWORD WeaponServices;
	inline DWORD MyWeapons;
	inline DWORD ActiveWeapon;

	// Entity identity (for projectile scanning)
	inline DWORD EntityIdentity;         // CEntityInstance::m_pEntity
	inline DWORD DesignerName;           // CEntityIdentity::m_designerName
	// Observer (spectator detection)
	inline DWORD ObserverServices;        // C_BasePlayerPawn::m_pObserverServices
	inline DWORD ObserverMode;             // CPlayer_ObserverServices::m_iObserverMode
	inline DWORD ObserverTarget;           // CPlayer_ObserverServices::m_hObserverTarget

	// Grenade projectile
	inline DWORD GrenadeIsLive;           // C_BaseGrenade::m_bIsLive
	inline DWORD GrenadeDmgRadius;        // C_BaseGrenade::m_DmgRadius
	inline DWORD GrenadeDetonateTime;     // C_BaseGrenade::m_flDetonateTime
	inline DWORD GrenadeThrower;          // C_BaseGrenade::m_hThrower
	inline DWORD ProjSpawnTime;           // C_BaseCSGrenadeProjectile::m_flSpawnTime
	inline DWORD ProjInitialVelocity;     // C_BaseCSGrenadeProjectile::m_vInitialVelocity
	inline DWORD ProjBounces;             // C_BaseCSGrenadeProjectile::m_nBounces

	struct
	{
		DWORD RealTime = 0x00;
		DWORD FrameCount = 0x04;
		DWORD MaxClients = 0x10;
		DWORD IntervalPerTick = 0x14;
		DWORD CurrentTime = 0x2C;
		DWORD CurrentTime2 = 0x30;
		DWORD TickCount = 0x40;
		DWORD IntervalPerTick2 = 0x44;
		DWORD CurrentNetchan = 0x0048;
		DWORD CurrentMap = 0x0180;
		DWORD CurrentMapName = 0x0188;
	} inline GlobalVar;

	bool UpdateOffsets(std::string offsetdata, std::string clientdata);
	bool ParseVersion(const std::string& versionData);
	bool CheckGameVersion(const std::string& steamNewsData);
}
