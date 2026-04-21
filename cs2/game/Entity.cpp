#include "Entity.h"
#include "../utils/Logger.h"

bool CEntity::UpdateController(const DWORD64& PlayerControllerAddress)
{
	if (PlayerControllerAddress == 0)
		return false;

	this->Controller.Address = PlayerControllerAddress;

	if (!this->Controller.GetHealth()) {
		LOG_TRACE("Entity", "Controller GetHealth FAIL");
		return false;
	}

	if (!this->Controller.GetArmor()) {
		LOG_TRACE("Entity", "Controller GetArmor FAIL");
		return false;
	}
	//if (!this->Controller.GetMoney()) {
	//	std::cout << "\t\GetMoney FAIL!" << std::endl;
	//	return false;
	//}

	if (!this->Controller.GetIsAlive()) {
		LOG_TRACE("Entity", "Controller IsAlive FAIL");
		return false;
	}

	if (!this->Controller.GetTeamID())
		return false;

	if (!this->Controller.GetPlayerName())
		return false;

	this->Pawn.Address = this->Controller.GetPlayerPawnAddress();

	return true;
}

bool CEntity::UpdatePawn(const DWORD64& PlayerPawnAddress)
{
	if (PlayerPawnAddress == 0)
		return false;

	this->Pawn.Address = PlayerPawnAddress;

	if (!this->Pawn.GetCameraPos())
	{
		LOG_TRACE("Entity", "Pawn GetCameraPos FAIL");
		return false;
	}
	if (!this->Pawn.GetPos()) {
		LOG_TRACE("Entity", "Pawn GetPos FAIL");
		return false;
	}
	if (!this->Pawn.GetViewAngle()) {
		LOG_TRACE("Entity", "Pawn GetViewAngle FAIL");
		return false;
	}
	if (!this->Pawn.GetWeaponName()) {
		LOG_TRACE("Entity", "Pawn GetWeaponName FAIL");
		return false;
	}
	if (!this->Pawn.GetAimPunchAngle()) {
		LOG_TRACE("Entity", "Pawn GetAimPunchAngle FAIL");
		return false;
	}
	if (!this->Pawn.GetShotsFired()) {
		LOG_TRACE("Entity", "Pawn GetShotsFired FAIL");
		return false;
	}
	if (!this->Pawn.GetHealth()) {
		LOG_TRACE("Entity", "Pawn GetHealth FAIL");
		return false;
	}
	if (!this->Pawn.GetTeamID()) {
		LOG_TRACE("Entity", "Pawn GetTeamID FAIL");
		return false;
	}
	if (!this->Pawn.GetFov()) {
		LOG_TRACE("Entity", "Pawn GetFov FAIL");
		return false;
	}
	if (!this->Pawn.GetSpotted()) {
		LOG_TRACE("Entity", "Pawn GetSpotted FAIL");
		return false;
	}
	if (!this->Pawn.GetFFlags()) {
		LOG_TRACE("Entity", "Pawn GetFFlags FAIL");
		return false;
	}

	if (!this->Pawn.BoneData.UpdateAllBoneData(PlayerPawnAddress)) {
		LOG_TRACE("Entity", "Pawn UpdateAllBoneData FAIL");
		return false;
	}

	return true;
}

bool PlayerController::GetTeamID()
{
	return GetDataAddressWithOffset<int>(Address, Offset::TeamID, this->TeamID);
}

bool PlayerController::GetHealth()
{
	return GetDataAddressWithOffset<int>(Address, Offset::Health, this->Health);
}


bool PlayerController::GetIsAlive()
{
	return GetDataAddressWithOffset<int>(Address, Offset::IsAlive, this->AliveStatus);
}

bool PlayerController::GetPlayerName()
{
	char Buffer[MAX_PATH]{};

	if (!ProcessMgr.ReadMemory(Address + Offset::iszPlayerName, Buffer, MAX_PATH))
		return false;

	this->PlayerName = Buffer;
	if (this->PlayerName.empty())
		this->PlayerName = "Name_None";

	return true;
}

bool PlayerController::GetArmor()
{
	return GetDataAddressWithOffset<int>(Address, Offset::Armor, this->Armor);
}

bool PlayerController::GetMoney()
{
	DWORD64 addr;
	GetDataAddressWithOffset<DWORD64>(Address, Offset::MoneyService, addr);
	return GetDataAddressWithOffset<int>(addr, 64, this->Money);
}

bool PlayerPawn::GetViewAngle()
{
	return GetDataAddressWithOffset<Vec2>(Address, Offset::angEyeAngles, this->ViewAngle);
}

bool PlayerPawn::GetCameraPos()
{
	return GetDataAddressWithOffset<Vec3>(Address, Offset::vecLastClipCameraPos, this->CameraPos);
}

bool PlayerPawn::GetSpotted()
{
	return GetDataAddressWithOffset<DWORD64>(Address, Offset::bSpottedByMask, this->bSpottedByMask);
}

bool PlayerPawn::GetWeaponName()
{
	try {
		DWORD64 WeaponServicesPtr = 0;
		DWORD ActiveWeaponHandle = 0;
		if (!ProcessMgr.ReadMemory<DWORD64>(Address + Offset::WeaponServices, WeaponServicesPtr) || WeaponServicesPtr == 0) {
			WeaponName = "Weapon_None";
			return true;
		}
		if (!ProcessMgr.ReadMemory<DWORD>(WeaponServicesPtr + Offset::ActiveWeapon, ActiveWeaponHandle) || ActiveWeaponHandle == 0) {
			WeaponName = "Weapon_None";
			return true;
		}

		DWORD64 WeaponNameAddress = 0;
		char Buffer[MAX_PATH]{};
		WeaponNameAddress = ProcessMgr.TraceAddress(gGame.GetEntityListAddress(), { 0x10, 8 * ((ActiveWeaponHandle & 0x7FFF) >> 9), 0x70 * (ActiveWeaponHandle & 0x1FF), 0x10, 0x20, 0x0 });
		if (WeaponNameAddress == 0) {
			WeaponName = "Weapon_None";
			return true;
		}

		if (!ProcessMgr.ReadMemory(WeaponNameAddress, Buffer, MAX_PATH)) {
			return false;
		}

		if (!memchr(Buffer, 0, MAX_PATH))
			WeaponName = "Weapon_None";
		else if (strlen(Buffer) == 0) {
			WeaponName = "Weapon_None";
		}
		else {
			WeaponName = std::string(Buffer);
			std::size_t Index = WeaponName.find("_");
			if (Index == std::string::npos || WeaponName.empty())
			{
				WeaponName = "Weapon_None";
			}
			else
			{
				WeaponName = WeaponName.substr(Index + 1, WeaponName.size() - Index - 1);
			}
		}
		return true;
	}
	catch (const std::exception& ex) {
		WeaponName = "Weapon_None";
		return true;
	}
}

bool PlayerPawn::GetShotsFired()
{
	return GetDataAddressWithOffset<DWORD>(Address, Offset::iShotsFired, this->ShotsFired);
}

bool PlayerPawn::GetAimPunchAngle()
{
	return GetDataAddressWithOffset<Vec2>(Address, Offset::aimPunchAngle, this->AimPunchAngle);
}

bool PlayerPawn::GetTeamID()
{
	return GetDataAddressWithOffset<int>(Address, Offset::iTeamNum, this->TeamID);
}

DWORD64 PlayerController::GetPlayerPawnAddress()
{
	DWORD64 EntityPawnListEntry = 0;
	DWORD64 EntityPawnAddress = 0;

	if (!GetDataAddressWithOffset<DWORD>(Address, Offset::PlayerPawn, this->Pawn))
		return 0;

	if (!ProcessMgr.ReadMemory<DWORD64>(gGame.GetEntityListAddress(), EntityPawnListEntry))
		return 0;

	if (!ProcessMgr.ReadMemory<DWORD64>(EntityPawnListEntry + 0x10 + 8 * ((Pawn & 0x7FFF) >> 9), EntityPawnListEntry))
		return 0;

	if (!ProcessMgr.ReadMemory<DWORD64>(EntityPawnListEntry + 0x70 * (Pawn & 0x1FF), EntityPawnAddress))
		return 0;

	return EntityPawnAddress;
}

bool PlayerPawn::GetPos()
{
	return GetDataAddressWithOffset<Vec3>(Address, Offset::Pos, this->Pos);
}

bool PlayerPawn::GetHealth()
{
	return GetDataAddressWithOffset<int>(Address, Offset::CurrentHealth, this->Health);
}

bool PlayerPawn::GetFov()
{
	DWORD64 CameraServices = 0;
	if (!ProcessMgr.ReadMemory<DWORD64>(Address + Offset::CameraServices, CameraServices))
		return false;
	return GetDataAddressWithOffset<int>(CameraServices, Offset::iFovStart, this->Fov);
}

bool PlayerPawn::GetFFlags()
{
	return GetDataAddressWithOffset<int>(Address, Offset::fFlags, this->fFlags);
}

bool CEntity::InitPawnAddress(const DWORD64& PlayerPawnAddress)
{
	if (PlayerPawnAddress == 0)
		return false;

	this->Pawn.Address = PlayerPawnAddress;

	// Only resolve BoneArrayAddress (2 reads) - scatter handles all dynamic fields
	if (!this->Pawn.BoneData.ResolveBoneAddress(PlayerPawnAddress))
		return false;

	return true;
}

bool CEntity::IsAlive() const
{
	return this->Controller.AliveStatus == 1 && this->Pawn.Health > 0;
}

bool CEntity::IsInScreen()
{
	return gGame.View.WorldToScreen(this->Pawn.Pos, this->Pawn.ScreenPos);
}

CBone CEntity::GetBone() const
{
	if (this->Pawn.Address == 0)
		return CBone{};
	return this->Pawn.BoneData;
}