#pragma once

#include "../game/Game.h"
#include "../game/Entity.h"

#include <mutex>
#include <shared_mutex>

// Bomb state for web radar
struct BombData
{
	float x = 0, y = 0, z = 0;
	float blowTime = 0;
	bool isPlanted = false;
	bool isDefused = false;
	bool isDefusing = false;
	float defuseTime = 0;
	DWORD carrierPawnHandle = 0;
};

// Grenade projectile types
enum GrenadeProjectileType : uint8_t
{
	PROJ_FLASH = 0,
	PROJ_SMOKE = 1,
	PROJ_HE = 2,
	PROJ_MOLOTOV = 3,
	PROJ_DECOY = 4,
	PROJ_UNKNOWN = 255
};

// In-flight grenade projectile data
struct GrenadeProjectile
{
	Vec3 Position;
	float EffectRadius;           // game-units radius for range circle
	GrenadeProjectileType Type;
	DWORD64 EntityAddr;           // for dedup across frames
	bool Alive = true;            // still found in entity list
	float DisappearTimer = 0.f;   // seconds since entity disappeared
	float StationaryTimer = 0.f;  // seconds since position stopped changing
};

// Unified snapshot: single lock protects all shared state
struct GameSnapshot
{
	float Matrix[4][4]{};
	CEntity LocalPlayer;
	std::vector<CEntity> Entities;
	char MapName[32]{};
	int LocalPlayerIndex = -1;
	BombData Bomb;
	std::vector<GrenadeProjectile> Projectiles;
};

namespace Cheats
{
	void Menu();
	void Run();

	// Single shared_mutex: writer takes unique_lock only for swap, readers take shared_lock
	inline std::shared_mutex SnapshotMutex;
	inline GameSnapshot Snapshot;
}
