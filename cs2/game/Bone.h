#pragma once
#include <vector>
#include "Game.h"

enum BONEINDEX : DWORD
{
	head=6,
	neck_0=5,
	spine_1=4,
	spine_2=2,
	pelvis=0,
	arm_upper_L=8,
	arm_lower_L=9,
	hand_L=10,
	arm_upper_R=13,
	arm_lower_R=14,
	hand_R=15,
	leg_upper_L=22,
	leg_lower_L=23,
	ankle_L=24,
	leg_upper_R=25,
	leg_lower_R=26,
	ankle_R=27,
};

struct BoneJointData
{
	Vec3 Pos;
	char pad[0x14];
};

struct BoneJointPos
{
	Vec3 Pos;
	Vec2 ScreenPos;
	bool IsVisible = false;
};

class CBone
{
private:
	DWORD64 EntityPawnAddress = 0;
public:
	DWORD64 BoneArrayAddress = 0;
	static constexpr int NUM_BONES = 30;
	BoneJointPos BonePosList[NUM_BONES]{};
	int BonePosCount = 0;

	// Lightweight: only resolves BoneArrayAddress (2 reads)
	bool ResolveBoneAddress(const DWORD64& EntityPawnAddress);

	// Full read: resolves address + reads all bone data + W2S (used by TriggerBot)
	bool UpdateAllBoneData(const DWORD64& EntityPawnAddress);
};

namespace BoneJointList
{
	inline constexpr DWORD Trunk[] = { neck_0, spine_2, pelvis };
	inline constexpr DWORD LeftArm[] = { neck_0, arm_upper_L, arm_lower_L, hand_L };
	inline constexpr DWORD RightArm[] = { neck_0, arm_upper_R, arm_lower_R, hand_R };
	inline constexpr DWORD LeftLeg[] = { pelvis, leg_upper_L, leg_lower_L, ankle_L };
	inline constexpr DWORD RightLeg[] = { pelvis, leg_upper_R, leg_lower_R, ankle_R };

	struct BoneChain {
		const DWORD* data;
		size_t count;
		const DWORD* begin() const { return data; }
		const DWORD* end() const { return data + count; }
	};

	inline const BoneChain List[] = {
		{ Trunk, std::size(Trunk) },
		{ LeftArm, std::size(LeftArm) },
		{ RightArm, std::size(RightArm) },
		{ LeftLeg, std::size(LeftLeg) },
		{ RightLeg, std::size(RightLeg) },
	};
}