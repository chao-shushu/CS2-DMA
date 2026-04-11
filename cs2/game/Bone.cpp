#include "Bone.h"

bool CBone::ResolveBoneAddress(const DWORD64& EntityPawnAddress) {
    if (EntityPawnAddress == 0)
        return false;

    this->EntityPawnAddress = EntityPawnAddress;

    DWORD64 GameSceneNode = 0;
    if (!ProcessMgr.ReadMemory<DWORD64>(EntityPawnAddress + Offset::GameSceneNode, GameSceneNode))
        return false;
    if (!ProcessMgr.ReadMemory<DWORD64>(GameSceneNode + Offset::BoneArray, this->BoneArrayAddress))
        return false;

    return this->BoneArrayAddress != 0;
}

bool CBone::UpdateAllBoneData(const DWORD64& EntityPawnAddress) {
    if (!ResolveBoneAddress(EntityPawnAddress))
        return false;

    BoneJointData BoneArray[NUM_BONES]{};
    if (!ProcessMgr.ReadMemory(this->BoneArrayAddress, BoneArray, NUM_BONES * sizeof(BoneJointData)))
        return false;

    BonePosCount = 0;
    for (int i = 0; i < NUM_BONES; i++) {
        Vec2 ScreenPos;
        bool IsVisible = gGame.View.WorldToScreen(BoneArray[i].Pos, ScreenPos);
        BonePosList[i] = { BoneArray[i].Pos, ScreenPos, IsVisible };
        BonePosCount++;
    }

    return BonePosCount > 0;
}