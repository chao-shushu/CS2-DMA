#pragma once
#include "Game.h"
#include "View.h"
#include "Bone.h"
#include "MemoryHelper.h"

struct C_UTL_VECTOR
{
	DWORD64 Count = 0;
	DWORD64 Data = 0;
};

class PlayerController
{
public:
	DWORD64 Address = 0;
	int TeamID = 0;
	int Health = 0;
	int Armor = 0;
	int AliveStatus = 0;
	int Money = 0;
	int Color = -1;
	DWORD Pawn = 0;
	std::string PlayerName;
public:
	bool GetTeamID();
	bool GetHealth();
	bool GetIsAlive();
	bool GetPlayerName();
	bool GetArmor();
	bool GetMoney();
	DWORD64 GetPlayerPawnAddress();
};

class PlayerPawn
{
public:
	enum class Flags
	{
		NONE,
		IN_AIR = 1 << 0
	};

	DWORD64 Address = 0;
	CBone BoneData;
	Vec2 ViewAngle;
	Vec3 Pos;
	Vec2 ScreenPos;
	bool ScreenPosValid = false;
	Vec3 CameraPos;
	std::string WeaponName;
	DWORD ShotsFired;
	Vec2 AimPunchAngle;
	C_UTL_VECTOR AimPunchCache;
	int Health;
	int TeamID;
	int Fov;
	DWORD64 bSpottedByMask;
	int fFlags;
	bool HasHelmet = false;
	bool HasDefuser = false;
	std::string ModelName;
	std::vector<std::string> WeaponList;
public:
	bool GetPos();
	bool GetViewAngle();
	bool GetCameraPos();
	bool GetWeaponName();
	bool GetShotsFired();
	bool GetAimPunchAngle();
	bool GetHealth();
	bool GetTeamID();
	bool GetFov();
	bool GetSpotted();
	bool GetFFlags();
	bool GetAimPunchCache();

	constexpr bool HasFlag(const Flags Flag) const noexcept {
		return fFlags & (int)Flag;
	}
};

class CEntity
{
public:
	PlayerController Controller;
	PlayerPawn Pawn;
	int LocalPlayerControllerIndex = 0;
public:
	bool UpdateController(const DWORD64& PlayerControllerAddress);

	// Full pawn read (used by TriggerBot and initial discovery)
	bool UpdatePawn(const DWORD64& PlayerPawnAddress);

	// Lightweight: only resolves PawnAddress + BoneArrayAddress for scatter
	bool InitPawnAddress(const DWORD64& PlayerPawnAddress);

	bool IsAlive() const;

	bool IsInScreen();

	CBone GetBone() const;
};