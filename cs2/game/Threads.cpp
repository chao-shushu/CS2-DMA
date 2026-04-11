#include "Threads.h"

#include "../render/GrenadeHelper.h"
#include "MenuConfig.h"
#include "../utils/Logger.h"

#include <winnt.h>
#include <thread>
#include <cmath>
#include <set>
#include <chrono>
#include <unordered_map>
#include <algorithm>

// =====================================================================
//  ConnectionThread — manages game process lifecycle
//
//  States:
//    SEARCHING_GAME    → try Attach("cs2.exe") every 1s
//    INITIALIZING_GAME → call InitAddress(), transition to RUNNING
//    RUNNING           → periodically check process alive
// =====================================================================

VOID ConnectionThread()
{
	int searchAttempts = 0;

	while (true)
	{
		AppState state = globalVars::gameState.load();

		switch (state)
		{
		case AppState::SEARCHING_GAME:
		{
			searchAttempts++;
			StatusCode status = ProcessMgr.Attach("cs2.exe");
			if (status == SUCCEED) {
				LOG_INFO("Connection", "Found cs2.exe (PID: {}) after {} attempts", ProcessMgr.ProcessID, searchAttempts);
				searchAttempts = 0;
				globalVars::gameState.store(AppState::INITIALIZING_GAME);
			} else {
				Sleep(1000);
			}
			break;
		}
		case AppState::INITIALIZING_GAME:
		{
			if (gGame.InitAddress()) {
				LOG_INFO("Connection", "Game addresses initialized");
				globalVars::gameState.store(AppState::RUNNING);
				LOG_INFO("Connection", "Game running - enjoy!");
			} else {
				LOG_WARNING("Connection", "Failed to init addresses, retrying...");
				ProcessMgr.Detach();
				globalVars::gameState.store(AppState::SEARCHING_GAME);
			}
			break;
		}
		case AppState::RUNNING:
		{
			if (!ProcessMgr.IsProcessAlive()) {
				LOG_WARNING("Connection", "Game process lost, searching again...");
				ProcessMgr.Detach();
				{
					std::unique_lock<std::shared_mutex> lock(Cheats::SnapshotMutex);
					Cheats::Snapshot = GameSnapshot{};
				}
				globalVars::gameState.store(AppState::SEARCHING_GAME);
			} else {
				Sleep(2000);
			}
			break;
		}
		default:
			Sleep(100);
			break;
		}
	}
}

// ---------- Validation helpers ----------

static bool IsValidPos(const Vec3& pos)
{
	return std::isfinite(pos.x) && std::isfinite(pos.y) && std::isfinite(pos.z)
		&& std::abs(pos.x) < 50000.f && std::abs(pos.y) < 50000.f && std::abs(pos.z) < 50000.f;
}

static bool IsValidHealth(int hp)
{
	return hp > 0 && hp <= 100;
}

// =====================================================================
//  DataThread — optimized data pipeline
//
//  Optimizations:
//    - Feature-gated scatter: only read fields needed by active menu features
//    - Entity caching: reuse controller data across frames, re-discover every N frames
//    - Dead field removal: spottedMask/aimPunch/shotsFired/fFlags/teamID removed from scatter
//    - Scattered entity discovery: 64 sequential reads → 1 scatter batch
//    - On-demand reading: skip entire pipeline when no features enabled
// =====================================================================

VOID DataThread()
{
	// --- Entity cache: persists across frames, avoids re-reading controller data ---
	struct CachedEntity {
		DWORD64 controllerAddr;
		DWORD64 pawnAddr;
		DWORD64 sceneNodeAddr;
		DWORD64 boneArrayAddr;
		CEntity entity;
	};
	constexpr int MAX_ENTITIES = 128;
	std::vector<CachedEntity> entityCache;
	entityCache.reserve(MAX_ENTITIES);

	CEntity localPlayer;
	DWORD64 localPawnAddrCached = 0;
	float matrix[4][4]{};
	int localPlayerIndex = -1;

	// Scatter buffers — only fields actually consumed by render
	struct ScatterBuf {
		BoneJointData bones[CBone::NUM_BONES]{};
		Vec3 pos;
		int health;
		Vec2 viewAngle;
		Vec3 cameraPos;
	};
	static ScatterBuf scatterBuf[MAX_ENTITIES];
	ScatterBuf localBuf{};

	int weaponCounter = 0;
	constexpr int WEAPON_UPDATE_INTERVAL = 5; // ~10ms at 2ms cycle

	// Tiered update frequency
	int frameCounter = 0;
	constexpr int DISCOVERY_INTERVAL = 5;      // re-discover entities every 5 frames (~10ms)
	constexpr int CONTROLLER_REFRESH = 50;     // refresh names/armor/teamID every 50 frames (~100ms)

	while (true)
	{
		try {
			Sleep(1);

			if (globalVars::gameState.load() != AppState::RUNNING) {
				Sleep(100);
				continue;
			}

			frameCounter++;

			// --- Compute feature needs from MenuConfig ---
			bool anyESPDraw = MenuConfig::ShowBoxESP || MenuConfig::ShowBoneESP ||
			                  MenuConfig::ShowHealthBar || MenuConfig::ShowWeaponESP ||
			                  MenuConfig::ShowPlayerName || MenuConfig::ShowDistance ||
			                  MenuConfig::ShowEyeRay || MenuConfig::ShowLineToEnemy ||
			                  MenuConfig::ShowHeadDot || MenuConfig::ShowArmorBar;
			bool needEntityPipeline = anyESPDraw || MenuConfig::ShowWebRadar;
			bool anyFeature = needEntityPipeline || MenuConfig::ShowProjectileESP;

			if (!anyFeature) {
				Sleep(50);
				std::unique_lock<std::shared_mutex> lock(Cheats::SnapshotMutex);
				Cheats::Snapshot.Entities.clear();
				continue;
			}

			// Get2DBox uses head bone → bones required for any ESP drawing
			bool needBones = anyESPDraw;
			bool needViewAngle = MenuConfig::ShowEyeRay || MenuConfig::ShowWebRadar || GrenadeHelper::Enabled;
			bool needCameraPos = MenuConfig::ShowEyeRay;
			bool needWeapon = MenuConfig::ShowWeaponESP || MenuConfig::ShowWebRadar;

			// ------- 1. Read matrix -------
			if (!ProcessMgr.ReadMemory(gGame.GetMatrixAddress(), matrix, 64))
				continue;
			memcpy(gGame.View.Matrix, matrix, 64);

			// ------- 2. Read local player addresses -------
			DWORD64 localControllerAddr = 0;
			DWORD64 localPawnAddr = 0;
			if (!ProcessMgr.ReadMemory(gGame.GetLocalControllerAddress(), localControllerAddr))
				continue;
			if (!ProcessMgr.ReadMemory(gGame.GetLocalPawnAddress(), localPawnAddr))
				continue;

			// Local player: full controller read only on address change or periodic refresh
			bool localChanged = (localPawnAddr != localPawnAddrCached);
			if (localChanged || frameCounter % CONTROLLER_REFRESH == 0) {
				CEntity newLocal;
				if (!newLocal.UpdateController(localControllerAddr)) {
					localPawnAddrCached = localPawnAddr; // prevent infinite retry
					continue;
				}
				if (localPawnAddr == 0 || !newLocal.InitPawnAddress(localPawnAddr)) {
					// Player dead or pawn invalid — mark health 0, keep processing
					localPlayer.Pawn.Health = 0;
					localPawnAddrCached = localPawnAddr;
				} else {
					// Carry over WR extra data to avoid flicker
					newLocal.Controller.Money = localPlayer.Controller.Money;
					newLocal.Controller.Color = localPlayer.Controller.Color;
					newLocal.Pawn.HasHelmet = localPlayer.Pawn.HasHelmet;
					newLocal.Pawn.HasDefuser = localPlayer.Pawn.HasDefuser;
					newLocal.Pawn.ModelName = localPlayer.Pawn.ModelName;
					newLocal.Pawn.WeaponName = localPlayer.Pawn.WeaponName;
					newLocal.Pawn.WeaponList = localPlayer.Pawn.WeaponList;
					localPlayer = newLocal;
					localPawnAddrCached = localPawnAddr;
				}
			}

			// ------- 3-7. Entity pipeline (skip when only projectile ESP is on) -------
			if (needEntityPipeline) {

			bool isDiscoveryFrame = (frameCounter % DISCOVERY_INTERVAL == 0) || entityCache.empty();

			if (isDiscoveryFrame) {
				DWORD64 listEntry = gGame.GetEntityListEntry();
				if (listEntry == 0) continue;

				// Scatter-read entity addresses at once
				DWORD64 entityAddresses[MAX_ENTITIES]{};
				{
					VMMDLL_SCATTER_HANDLE addrHandle = ProcessMgr.CreateScatterHandle();
					if (!addrHandle) continue;
					for (int i = 0; i < MAX_ENTITIES; i++)
						ProcessMgr.AddScatterReadRequest(addrHandle, listEntry + (i + 1) * 0x70, &entityAddresses[i], sizeof(DWORD64));
					ProcessMgr.ExecuteReadScatter(addrHandle);
					VMMDLL_Scatter_CloseHandle(addrHandle);
				}

				bool controllerRefresh = (frameCounter % CONTROLLER_REFRESH == 0);
				std::vector<CachedEntity> newCache;
				newCache.reserve(MAX_ENTITIES);
				localPlayerIndex = -1;

				// Identify entities needing refresh vs cached
				struct { int health; int armor; int isAlive; int teamID; DWORD pawn; char name[MAX_PATH]; } ctrlBuf[MAX_ENTITIES]{};
				int refreshSlots[MAX_ENTITIES]{};
				int refreshCount = 0;

				for (int i = 0; i < MAX_ENTITIES; i++) {
					DWORD64 entityAddr = entityAddresses[i];
					if (entityAddr == 0) continue;
					if (entityAddr == localControllerAddr) {
						localPlayerIndex = i;
						continue;
					}

					CachedEntity* existing = nullptr;
					for (auto& ce : entityCache) {
						if (ce.controllerAddr == entityAddr) { existing = &ce; break; }
					}

					if (existing && !controllerRefresh) {
						newCache.push_back(*existing);
					} else {
						refreshSlots[refreshCount++] = i;
					}
				}

				// Phase 1: Scatter-read controller fields (batched to avoid scatter page limit)
				constexpr int CTRL_SCATTER_BATCH = 8;
				if (refreshCount > 0) {
					for (int batchStart = 0; batchStart < refreshCount; batchStart += CTRL_SCATTER_BATCH) {
						int batchEnd = (batchStart + CTRL_SCATTER_BATCH < refreshCount) ? batchStart + CTRL_SCATTER_BATCH : refreshCount;
						VMMDLL_SCATTER_HANDLE h = ProcessMgr.CreateScatterHandle();
						if (!h) continue;
						for (int r = batchStart; r < batchEnd; r++) {
							int i = refreshSlots[r];
							DWORD64 addr = entityAddresses[i];
							ProcessMgr.AddScatterReadRequest(h, addr + Offset::Health, &ctrlBuf[i].health, sizeof(int));
							ProcessMgr.AddScatterReadRequest(h, addr + Offset::Armor, &ctrlBuf[i].armor, sizeof(int));
							ProcessMgr.AddScatterReadRequest(h, addr + Offset::IsAlive, &ctrlBuf[i].isAlive, sizeof(int));
							ProcessMgr.AddScatterReadRequest(h, addr + Offset::TeamID, &ctrlBuf[i].teamID, sizeof(int));
							ProcessMgr.AddScatterReadRequest(h, addr + Offset::PlayerPawn, &ctrlBuf[i].pawn, sizeof(DWORD));
							ProcessMgr.AddScatterReadRequest(h, addr + Offset::iszPlayerName, ctrlBuf[i].name, MAX_PATH);
						}
						ProcessMgr.ExecuteReadScatter(h);
						VMMDLL_Scatter_CloseHandle(h);
					}

					// Phase 2: Resolve pawn list sub-entries for alive entities
					DWORD64 entityPawnListEntry = 0;
					ProcessMgr.ReadMemory<DWORD64>(gGame.GetEntityListAddress(), entityPawnListEntry);

					DWORD64 subListEntries[MAX_ENTITIES]{};
					int aliveSlots[MAX_ENTITIES]{};
					int aliveCount = 0;

					if (entityPawnListEntry != 0) {
						VMMDLL_SCATTER_HANDLE h = ProcessMgr.CreateScatterHandle();
						if (h) {
							for (int r = 0; r < refreshCount; r++) {
								int i = refreshSlots[r];
								if (ctrlBuf[i].isAlive != 1 || ctrlBuf[i].pawn == 0) continue;
								aliveSlots[aliveCount++] = i;
								ProcessMgr.AddScatterReadRequest(h, entityPawnListEntry + 0x10 + 8 * ((ctrlBuf[i].pawn & 0x7FFF) >> 9), &subListEntries[i], sizeof(DWORD64));
							}
							ProcessMgr.ExecuteReadScatter(h);
							VMMDLL_Scatter_CloseHandle(h);
						}
					}

					// Phase 3: Final pawn addresses
					DWORD64 pawnAddresses[MAX_ENTITIES]{};
					{
						VMMDLL_SCATTER_HANDLE h = ProcessMgr.CreateScatterHandle();
						if (h) {
							for (int a = 0; a < aliveCount; a++) {
								int i = aliveSlots[a];
								if (subListEntries[i] == 0) continue;
								ProcessMgr.AddScatterReadRequest(h, subListEntries[i] + 0x70 * (ctrlBuf[i].pawn & 0x1FF), &pawnAddresses[i], sizeof(DWORD64));
							}
							ProcessMgr.ExecuteReadScatter(h);
							VMMDLL_Scatter_CloseHandle(h);
						}
					}

					// Phase 4+5: GameSceneNode → BoneArray (batched, 1 page per entity)
					constexpr int DISC_BATCH = 6;
					DWORD64 sceneNodes[MAX_ENTITIES]{};
					DWORD64 boneArrays[MAX_ENTITIES]{};
					{
						VMMDLL_SCATTER_HANDLE h = nullptr;
						int bc = 0;
						for (int a = 0; a < aliveCount; a++) {
							int i = aliveSlots[a];
							if (pawnAddresses[i] == 0) continue;
							if (!h) { h = ProcessMgr.CreateScatterHandle(); if (!h) continue; bc = 0; }
							ProcessMgr.AddScatterReadRequest(h, pawnAddresses[i] + Offset::GameSceneNode, &sceneNodes[i], sizeof(DWORD64));
							if (++bc >= DISC_BATCH) {
								ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); h = nullptr; bc = 0;
							}
						}
						if (h) { ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); }
					}
					{
						VMMDLL_SCATTER_HANDLE h = nullptr;
						int bc = 0;
						for (int a = 0; a < aliveCount; a++) {
							int i = aliveSlots[a];
							if (sceneNodes[i] == 0) continue;
							if (!h) { h = ProcessMgr.CreateScatterHandle(); if (!h) continue; bc = 0; }
							ProcessMgr.AddScatterReadRequest(h, sceneNodes[i] + Offset::BoneArray, &boneArrays[i], sizeof(DWORD64));
							if (++bc >= DISC_BATCH) {
								ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); h = nullptr; bc = 0;
							}
						}
						if (h) { ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); }
					}

					// Build CachedEntity from scatter results
					for (int a = 0; a < aliveCount; a++) {
						int i = aliveSlots[a];
						if (pawnAddresses[i] == 0 || boneArrays[i] == 0)
							continue;

						CEntity ent;
						ent.Controller.Address = entityAddresses[i];
						ent.Controller.Health = ctrlBuf[i].health;
						ent.Controller.Armor = ctrlBuf[i].armor;
						ent.Controller.AliveStatus = ctrlBuf[i].isAlive;
						ent.Controller.TeamID = ctrlBuf[i].teamID;
						ent.Controller.Pawn = ctrlBuf[i].pawn;
						if (memchr(ctrlBuf[i].name, 0, MAX_PATH) && strlen(ctrlBuf[i].name) > 0)
							ent.Controller.PlayerName = ctrlBuf[i].name;
						else
							ent.Controller.PlayerName = "Name_None";

						ent.Pawn.Address = pawnAddresses[i];
						ent.Pawn.BoneData.BoneArrayAddress = boneArrays[i];

						CachedEntity ce;
						ce.controllerAddr = entityAddresses[i];
						ce.pawnAddr = pawnAddresses[i];
						ce.sceneNodeAddr = sceneNodes[i];
						ce.boneArrayAddr = boneArrays[i];
						ce.entity = ent;

						// Carry over WR extra data from previous cache to avoid flicker
						for (const auto& old : entityCache) {
							if (old.controllerAddr == entityAddresses[i]) {
								ce.entity.Controller.Money = old.entity.Controller.Money;
								ce.entity.Controller.Color = old.entity.Controller.Color;
								ce.entity.Pawn.HasHelmet = old.entity.Pawn.HasHelmet;
								ce.entity.Pawn.HasDefuser = old.entity.Pawn.HasDefuser;
								ce.entity.Pawn.ModelName = old.entity.Pawn.ModelName;
								ce.entity.Pawn.WeaponName = old.entity.Pawn.WeaponName;
								ce.entity.Pawn.WeaponList = old.entity.Pawn.WeaponList;
								break;
							}
						}

						newCache.push_back(ce);
					}

					// Retain dead entities from previous cache for WebRadar continuity.
					// Without this, C4 killing everyone → empty cache → empty m_players → "waiting for data".
					for (int r = 0; r < refreshCount; r++) {
						int i = refreshSlots[r];
						DWORD64 addr = entityAddresses[i];
						// Skip if already added (alive and resolved above)
						bool alreadyAdded = false;
						for (const auto& nc : newCache) {
							if (nc.controllerAddr == addr) { alreadyAdded = true; break; }
						}
						if (alreadyAdded) continue;
						// Carry forward from old cache with health zeroed
						for (const auto& old : entityCache) {
							if (old.controllerAddr == addr) {
								CachedEntity copy = old;
								copy.entity.Pawn.Health = 0;
								copy.entity.Controller.Health = 0;
								newCache.push_back(copy);
								break;
							}
						}
					}
				}

				entityCache = std::move(newCache);
			}

			// ------- 4. Scatter read dynamic fields (2-pass: refresh bone addresses every frame) -------
			{
				int count = (int)entityCache.size();
				if (count > MAX_ENTITIES) count = MAX_ENTITIES;

				// --- Pass 1: pos, health, viewAngle, cameraPos + fresh BoneArray pointer ---
				// Pawn offsets span 3 pages: health@0x354(page0), pos@0x1588(page1), eyeAngles@0x3DD0(page3)
				// Plus sceneNode BoneArray@0x1E0 = 4 unique pages per entity. Batch=2 → 8 pages.
				DWORD64 freshBoneArrays[MAX_ENTITIES]{};
				{
					for (int i = 0; i < count; i++)
						memset(&scatterBuf[i], 0, sizeof(ScatterBuf));
					memset(&localBuf, 0, sizeof(ScatterBuf));

					constexpr int PASS1_BATCH = 2; // pawn spans 3 pages (0x354,0x1588,0x3DD0) + sceneNode = 4 pages/entity
					for (int batchStart = 0; batchStart < count; batchStart += PASS1_BATCH) {
						int batchEnd = (batchStart + PASS1_BATCH < count) ? batchStart + PASS1_BATCH : count;
						VMMDLL_SCATTER_HANDLE handle = ProcessMgr.CreateScatterHandle();
						if (!handle) continue;
						for (int i = batchStart; i < batchEnd; i++) {
							auto& ce = entityCache[i];
							auto& buf = scatterBuf[i];
							DWORD64 pawn = ce.pawnAddr;
							ProcessMgr.AddScatterReadRequest(handle, pawn + Offset::Pos, &buf.pos, sizeof(Vec3));
							ProcessMgr.AddScatterReadRequest(handle, pawn + Offset::CurrentHealth, &buf.health, sizeof(int));
							if (needBones && ce.sceneNodeAddr != 0)
								ProcessMgr.AddScatterReadRequest(handle, ce.sceneNodeAddr + Offset::BoneArray, &freshBoneArrays[i], sizeof(DWORD64));
							if (needViewAngle)
								ProcessMgr.AddScatterReadRequest(handle, pawn + Offset::angEyeAngles, &buf.viewAngle, sizeof(Vec2));
							if (needCameraPos)
								ProcessMgr.AddScatterReadRequest(handle, pawn + Offset::vecLastClipCameraPos, &buf.cameraPos, sizeof(Vec3));
						}
						ProcessMgr.ExecuteReadScatter(handle);
						VMMDLL_Scatter_CloseHandle(handle);
					}

					// Local player dynamic fields (1 page, separate to avoid skip when count==0)
					{
						VMMDLL_SCATTER_HANDLE handle = ProcessMgr.CreateScatterHandle();
						if (handle) {
							DWORD64 lp = localPlayer.Pawn.Address;
							ProcessMgr.AddScatterReadRequest(handle, lp + Offset::Pos, &localBuf.pos, sizeof(Vec3));
							ProcessMgr.AddScatterReadRequest(handle, lp + Offset::CurrentHealth, &localBuf.health, sizeof(int));
							if (needViewAngle)
								ProcessMgr.AddScatterReadRequest(handle, lp + Offset::angEyeAngles, &localBuf.viewAngle, sizeof(Vec2));
							ProcessMgr.ExecuteReadScatter(handle);
							VMMDLL_Scatter_CloseHandle(handle);
						}
					}
				}

				// Update bone array addresses from fresh reads
				for (int i = 0; i < count; i++) {
					if (freshBoneArrays[i] != 0) {
						entityCache[i].boneArrayAddr = freshBoneArrays[i];
						entityCache[i].entity.Pawn.BoneData.BoneArrayAddress = freshBoneArrays[i];
					}
				}

				// --- Pass 2: bone data from fresh addresses ---
				// VMMDLL scatter silently drops reads when too many unique pages
				// are batched in one ExecuteRead (~8 page limit observed).
				// Split bone reads into batches to stay within the limit.
				constexpr int BONE_SCATTER_BATCH = 6;
				if (needBones) {
					for (int batchStart = 0; batchStart < count; batchStart += BONE_SCATTER_BATCH) {
						int batchEnd = (batchStart + BONE_SCATTER_BATCH < count) ? batchStart + BONE_SCATTER_BATCH : count;
						VMMDLL_SCATTER_HANDLE handle = ProcessMgr.CreateScatterHandle();
						if (!handle) continue;
						for (int i = batchStart; i < batchEnd; i++) {
							if (entityCache[i].boneArrayAddr != 0)
								ProcessMgr.AddScatterReadRequest(handle, entityCache[i].boneArrayAddr, scatterBuf[i].bones, CBone::NUM_BONES * sizeof(BoneJointData));
						}
						ProcessMgr.ExecuteReadScatter(handle);
						VMMDLL_Scatter_CloseHandle(handle);
					}
				}

				// ------- 5. Apply scatter results (world coords only) -------
				// W2S moved to render thread: each render frame reads a fresh ViewMatrix
				// so ESP tracks view rotation at display refresh rate, not DMA update rate.
				for (int i = 0; i < count; i++)
				{
					auto& ce = entityCache[i];
					auto& buf = scatterBuf[i];

					if (!IsValidPos(buf.pos) || !IsValidHealth(buf.health)) {
						ce.entity.Pawn.Health = 0;
						ce.entity.Pawn.ScreenPosValid = false;
					} else {
						ce.entity.Pawn.Pos = buf.pos;
						ce.entity.Pawn.Health = buf.health;
						ce.entity.Pawn.ScreenPosValid = true; // render thread will refine via W2S

						if (needViewAngle)
							ce.entity.Pawn.ViewAngle = buf.viewAngle;
						if (needCameraPos)
							ce.entity.Pawn.CameraPos = buf.cameraPos;

						// Store bone world positions (W2S done in render thread)
						if (needBones) {
							ce.entity.Pawn.BoneData.BonePosCount = 0;
							for (int j = 0; j < CBone::NUM_BONES; j++)
							{
								const Vec3& bonePos = buf.bones[j].Pos;
								bool valid = std::isfinite(bonePos.x) && std::isfinite(bonePos.y) && std::isfinite(bonePos.z);
								ce.entity.Pawn.BoneData.BonePosList[j] = { bonePos, {0,0}, valid };
								ce.entity.Pawn.BoneData.BonePosCount++;
							}
						}
					}
				}

				// Apply local player scatter results
				if (IsValidPos(localBuf.pos)) {
					localPlayer.Pawn.Pos = localBuf.pos;
					localPlayer.Pawn.Health = localBuf.health;
					if (needViewAngle)
						localPlayer.Pawn.ViewAngle = localBuf.viewAngle;
				}
			}

			// ------- 6. Weapon names (low frequency, only if feature needs it) -------
			if (needWeapon) {
				weaponCounter++;
				if (weaponCounter >= WEAPON_UPDATE_INTERVAL) {
					weaponCounter = 0;
					localPlayer.Pawn.GetWeaponName();
					for (auto& ce : entityCache) {
						if (ce.pawnAddr != 0 && ce.entity.Pawn.Health > 0)
							ce.entity.Pawn.GetWeaponName();
					}
				} else {
					std::shared_lock<std::shared_mutex> readLock(Cheats::SnapshotMutex);
					localPlayer.Pawn.WeaponName = Cheats::Snapshot.LocalPlayer.Pawn.WeaponName;
					for (auto& ce : entityCache) {
						for (const auto& old : Cheats::Snapshot.Entities) {
							if (old.Controller.Address == ce.controllerAddr) {
								ce.entity.Pawn.WeaponName = old.Pawn.WeaponName;
								break;
							}
						}
					}
				}
			}

			// ------- 7. Web Radar extra data -------
			if (MenuConfig::ShowWebRadar) {
				static int wrExtraCounter = 0;
				static int wrSlowCounter = 0;
				if (++wrExtraCounter >= 25) { // ~50ms
					wrExtraCounter = 0;
					int cnt = (int)entityCache.size();
					if (cnt > MAX_ENTITIES) cnt = MAX_ENTITIES;

					// Phase A: scatter-read pointer fields + color
					DWORD64 moneyPtrs[MAX_ENTITIES]{};
					DWORD64 itemPtrs[MAX_ENTITIES]{};
					int colorBuf[MAX_ENTITIES]{};
					DWORD64 localMoneyPtr = 0;
					DWORD64 localItemPtr = 0;
					int localColor = -1;

					constexpr int WR_BATCH = 4;
					for (int batchStart = 0; batchStart < cnt; batchStart += WR_BATCH) {
						int batchEnd = (batchStart + WR_BATCH < cnt) ? batchStart + WR_BATCH : cnt;
						VMMDLL_SCATTER_HANDLE h = ProcessMgr.CreateScatterHandle();
						if (!h) continue;
						for (int i = batchStart; i < batchEnd; i++) {
							auto& ce = entityCache[i];
							if (ce.entity.Pawn.Health <= 0) continue;
							ProcessMgr.AddScatterReadRequest(h, ce.controllerAddr + Offset::MoneyService, &moneyPtrs[i], sizeof(DWORD64));
							ProcessMgr.AddScatterReadRequest(h, ce.pawnAddr + Offset::ItemServices, &itemPtrs[i], sizeof(DWORD64));
							ProcessMgr.AddScatterReadRequest(h, ce.controllerAddr + Offset::CompTeammateColor, &colorBuf[i], sizeof(int));
						}
						if (batchStart == 0 && localPlayer.Controller.Address != 0) {
							ProcessMgr.AddScatterReadRequest(h, localPlayer.Controller.Address + Offset::MoneyService, &localMoneyPtr, sizeof(DWORD64));
							ProcessMgr.AddScatterReadRequest(h, localPlayer.Pawn.Address + Offset::ItemServices, &localItemPtr, sizeof(DWORD64));
							ProcessMgr.AddScatterReadRequest(h, localPlayer.Controller.Address + Offset::CompTeammateColor, &localColor, sizeof(int));
						}
						ProcessMgr.ExecuteReadScatter(h);
						VMMDLL_Scatter_CloseHandle(h);
					}

					// Phase B: scatter-read values from resolved pointers
					int moneyBuf[MAX_ENTITIES]{};
					uint8_t helmetBuf[MAX_ENTITIES]{};
					uint8_t defuserBuf[MAX_ENTITIES]{};
					int localMoney = 0;
					uint8_t localHelmet = 0, localDefuser = 0;

					for (int batchStart = 0; batchStart < cnt; batchStart += WR_BATCH) {
						int batchEnd = (batchStart + WR_BATCH < cnt) ? batchStart + WR_BATCH : cnt;
						VMMDLL_SCATTER_HANDLE h = ProcessMgr.CreateScatterHandle();
						if (!h) continue;
						for (int i = batchStart; i < batchEnd; i++) {
							if (moneyPtrs[i])
								ProcessMgr.AddScatterReadRequest(h, moneyPtrs[i] + 64, &moneyBuf[i], sizeof(int));
							if (itemPtrs[i]) {
								ProcessMgr.AddScatterReadRequest(h, itemPtrs[i] + Offset::HasDefuser, &defuserBuf[i], sizeof(uint8_t));
								ProcessMgr.AddScatterReadRequest(h, itemPtrs[i] + Offset::HasHelmet, &helmetBuf[i], sizeof(uint8_t));
							}
						}
						if (batchStart == 0) {
							if (localMoneyPtr)
								ProcessMgr.AddScatterReadRequest(h, localMoneyPtr + 64, &localMoney, sizeof(int));
							if (localItemPtr) {
								ProcessMgr.AddScatterReadRequest(h, localItemPtr + Offset::HasDefuser, &localDefuser, sizeof(uint8_t));
								ProcessMgr.AddScatterReadRequest(h, localItemPtr + Offset::HasHelmet, &localHelmet, sizeof(uint8_t));
							}
						}
						ProcessMgr.ExecuteReadScatter(h);
						VMMDLL_Scatter_CloseHandle(h);
					}

					// Apply player results
					for (int i = 0; i < cnt; i++) {
						entityCache[i].entity.Controller.Money = moneyBuf[i];
						entityCache[i].entity.Controller.Color = colorBuf[i];
						entityCache[i].entity.Pawn.HasHelmet = helmetBuf[i] != 0;
						entityCache[i].entity.Pawn.HasDefuser = defuserBuf[i] != 0;
					}
					localPlayer.Controller.Money = localMoney;
					localPlayer.Controller.Color = localColor;
					localPlayer.Pawn.HasHelmet = localHelmet != 0;
					localPlayer.Pawn.HasDefuser = localDefuser != 0;

					// --- Bomb data (every ~50ms) ---
					BombData bombSnap{};
					DWORD64 clientBase = gGame.GetClientDLLAddress();

					// Planted bomb — dwPlantedC4 is a CUtlVector data ptr, need:
					// 1. Check count byte at (client + dwPlantedC4 - 8)
					// 2. Double deref: *(*(client + dwPlantedC4)) = entity
					DWORD64 plantedEntity = 0;
					uint8_t plantedCount = 0;
					ProcessMgr.ReadMemory<uint8_t>(clientBase + Offset::PlantedC4 - 8, plantedCount);
					if (plantedCount > 0) {
						DWORD64 listDataPtr = 0;
						if (ProcessMgr.ReadMemory<DWORD64>(clientBase + Offset::PlantedC4, listDataPtr) && listDataPtr) {
							ProcessMgr.ReadMemory<DWORD64>(listDataPtr, plantedEntity);
						}
					}
					if (plantedEntity && (plantedEntity >> 48) == 0) {
						uint8_t ticking = 0;
						ProcessMgr.ReadMemory<uint8_t>(plantedEntity + Offset::BombTicking, ticking);
						if (ticking) {
							bombSnap.isPlanted = true;
							// Read bomb fields in one scatter
							float blowTime = 0, defuseCD = 0;
							uint8_t defused = 0, defusing = 0;
							DWORD64 sceneNode = 0;
							{
								VMMDLL_SCATTER_HANDLE h = ProcessMgr.CreateScatterHandle();
								if (h) {
									ProcessMgr.AddScatterReadRequest(h, plantedEntity + Offset::C4Blow, &blowTime, sizeof(float));
									ProcessMgr.AddScatterReadRequest(h, plantedEntity + Offset::BombDefused, &defused, sizeof(uint8_t));
									ProcessMgr.AddScatterReadRequest(h, plantedEntity + Offset::BeingDefused, &defusing, sizeof(uint8_t));
									ProcessMgr.AddScatterReadRequest(h, plantedEntity + Offset::DefuseCountDown, &defuseCD, sizeof(float));
									ProcessMgr.AddScatterReadRequest(h, plantedEntity + Offset::GameSceneNode, &sceneNode, sizeof(DWORD64));
									ProcessMgr.ExecuteReadScatter(h);
									VMMDLL_Scatter_CloseHandle(h);
								}
							}
							// Read curtime from GlobalVars (offset 0x30 = CurrentTime2, confirmed by RE)
							DWORD64 gvPtr = 0;
							float curtime = 0;
							if (ProcessMgr.ReadMemory<DWORD64>(clientBase + Offset::GlobalVars, gvPtr) && gvPtr) {
								ProcessMgr.ReadMemory<float>(gvPtr + Offset::GlobalVar.CurrentTime2, curtime);
							}
							bombSnap.blowTime = blowTime - curtime;
							if (bombSnap.blowTime < 0) bombSnap.blowTime = 0;
							bombSnap.isDefused = defused != 0;
							bombSnap.isDefusing = defusing != 0;
							bombSnap.defuseTime = defuseCD - curtime;
							if (bombSnap.defuseTime < 0) bombSnap.defuseTime = 0;

							// Read position from scene node
							if (sceneNode && (sceneNode >> 48) == 0) { // sanity: valid usermode ptr
								Vec3 bombPos{};
								ProcessMgr.ReadMemory(sceneNode + Offset::vecAbsOrigin, bombPos);
								bombSnap.x = bombPos.x;
								bombSnap.y = bombPos.y;
								bombSnap.z = bombPos.z;
							}
						}
					}

					// Carried bomb: scan weapon lists for "c4"
					if (!bombSnap.isPlanted) {
						// Check local player
						for (const auto& w : localPlayer.Pawn.WeaponList) {
							if (w == "c4") {
								bombSnap.carrierPawnHandle = localPlayer.Controller.Pawn;
								bombSnap.x = localPlayer.Pawn.Pos.x;
								bombSnap.y = localPlayer.Pawn.Pos.y;
								bombSnap.z = localPlayer.Pawn.Pos.z;
								break;
							}
						}
						// Check other entities
						if (bombSnap.carrierPawnHandle == 0) {
							for (const auto& ce : entityCache) {
								for (const auto& w : ce.entity.Pawn.WeaponList) {
									if (w == "c4") {
										bombSnap.carrierPawnHandle = ce.entity.Controller.Pawn;
										bombSnap.x = ce.entity.Pawn.Pos.x;
										bombSnap.y = ce.entity.Pawn.Pos.y;
										bombSnap.z = ce.entity.Pawn.Pos.z;
										break;
									}
								}
								if (bombSnap.carrierPawnHandle != 0) break;
							}
						}
					}

					// Dropped C4: dwWeaponC4 has same CUtlVector layout as dwPlantedC4
					if (!bombSnap.isPlanted && bombSnap.carrierPawnHandle == 0) {
						DWORD64 weaponEntity = 0;
						DWORD64 weaponListPtr = 0;
						if (ProcessMgr.ReadMemory<DWORD64>(clientBase + Offset::WeaponC4, weaponListPtr) && weaponListPtr) {
							ProcessMgr.ReadMemory<DWORD64>(weaponListPtr, weaponEntity);
						}
						if (weaponEntity && (weaponEntity >> 48) == 0) {
							DWORD64 sn = 0;
							ProcessMgr.ReadMemory<DWORD64>(weaponEntity + Offset::GameSceneNode, sn);
							if (sn && (sn >> 48) == 0) {
								Vec3 dropPos{};
								ProcessMgr.ReadMemory(sn + Offset::vecAbsOrigin, dropPos);
								bombSnap.x = dropPos.x;
								bombSnap.y = dropPos.y;
								bombSnap.z = dropPos.z;
							}
						}
					}


					// Store bomb data for snapshot
					{
						std::unique_lock<std::shared_mutex> lock(Cheats::SnapshotMutex);
						Cheats::Snapshot.Bomb = bombSnap;
					}
				}

				// --- Model name + full weapon list (low frequency ~5s) ---
				if (++wrSlowCounter >= 100) {
					wrSlowCounter = 0;
					int cnt = (int)entityCache.size();
					if (cnt > MAX_ENTITIES) cnt = MAX_ENTITIES;

					// ========= Model Name =========
					constexpr int MODEL_BATCH = 6; // 1 page per entity
					// Phase M1: read GameSceneNode pointers (batched)
					DWORD64 sceneNodes[MAX_ENTITIES]{};
					DWORD64 localSceneNode = 0;
					{
						VMMDLL_SCATTER_HANDLE h = nullptr;
						int bc = 0;
						for (int i = 0; i < cnt; i++) {
							if (!entityCache[i].pawnAddr) continue;
							if (!h) { h = ProcessMgr.CreateScatterHandle(); if (!h) continue; bc = 0; }
							ProcessMgr.AddScatterReadRequest(h, entityCache[i].pawnAddr + Offset::GameSceneNode, &sceneNodes[i], sizeof(DWORD64));
							if (++bc >= MODEL_BATCH) {
								ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); h = nullptr; bc = 0;
							}
						}
						if (localPlayer.Pawn.Address) {
							if (!h) { h = ProcessMgr.CreateScatterHandle(); }
							if (h) ProcessMgr.AddScatterReadRequest(h, localPlayer.Pawn.Address + Offset::GameSceneNode, &localSceneNode, sizeof(DWORD64));
						}
						if (h) { ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); }
					}

					// Phase M2: read m_ModelName pointer (batched)
					DWORD64 nameAddrs[MAX_ENTITIES]{};
					DWORD64 localNameAddr = 0;
					{
						VMMDLL_SCATTER_HANDLE h = nullptr;
						int bc = 0;
						for (int i = 0; i < cnt; i++) {
							if (!sceneNodes[i]) continue;
							if (!h) { h = ProcessMgr.CreateScatterHandle(); if (!h) continue; bc = 0; }
							ProcessMgr.AddScatterReadRequest(h, sceneNodes[i] + Offset::ModelStateOffset + Offset::ModelNameOffset, &nameAddrs[i], sizeof(DWORD64));
							if (++bc >= MODEL_BATCH) {
								ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); h = nullptr; bc = 0;
							}
						}
						if (localSceneNode) {
							if (!h) { h = ProcessMgr.CreateScatterHandle(); }
							if (h) ProcessMgr.AddScatterReadRequest(h, localSceneNode + Offset::ModelStateOffset + Offset::ModelNameOffset, &localNameAddr, sizeof(DWORD64));
						}
						if (h) { ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); }
					}

					// Phase M3: read model name strings
					auto readModelStr = [](DWORD64 addr) -> std::string {
						if (!addr) return "";
						char buf[128]{};
						ProcessMgr.ReadMemory(addr, buf, sizeof(buf) - 1);
						buf[127] = '\0';
						std::string s(buf);
						auto slash = s.rfind('/');
						if (slash != std::string::npos) s = s.substr(slash + 1);
						auto dot = s.rfind('.');
						if (dot != std::string::npos) s = s.substr(0, dot);
						return s;
					};

					for (int i = 0; i < cnt; i++)
						entityCache[i].entity.Pawn.ModelName = readModelStr(nameAddrs[i]);
					localPlayer.Pawn.ModelName = readModelStr(localNameAddr);

					// ========= Full Weapon List =========
					constexpr int MAX_WEAPONS_PER_PLAYER = 10;

					constexpr int WEP_BATCH = 6; // 1 page per entity
					// Phase W1: read WeaponServices pointers (batched)
					DWORD64 wsPtrs[MAX_ENTITIES]{};
					DWORD64 localWsPtr = 0;
					{
						VMMDLL_SCATTER_HANDLE h = nullptr;
						int bc = 0;
						for (int i = 0; i < cnt; i++) {
							if (!entityCache[i].pawnAddr || entityCache[i].entity.Pawn.Health <= 0) continue;
							if (!h) { h = ProcessMgr.CreateScatterHandle(); if (!h) continue; bc = 0; }
							ProcessMgr.AddScatterReadRequest(h, entityCache[i].pawnAddr + Offset::WeaponServices, &wsPtrs[i], sizeof(DWORD64));
							if (++bc >= WEP_BATCH) {
								ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); h = nullptr; bc = 0;
							}
						}
						if (localPlayer.Pawn.Address) {
							if (!h) { h = ProcessMgr.CreateScatterHandle(); }
							if (h) ProcessMgr.AddScatterReadRequest(h, localPlayer.Pawn.Address + Offset::WeaponServices, &localWsPtr, sizeof(DWORD64));
						}
						if (h) { ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); }
					}

					// Phase W2: read m_hMyWeapons vector (batched)
					struct WepVec { int count; int pad; DWORD64 dataPtr; };
					WepVec wvBuf[MAX_ENTITIES]{};
					WepVec localWv{};
					{
						VMMDLL_SCATTER_HANDLE h = nullptr;
						int bc = 0;
						for (int i = 0; i < cnt; i++) {
							if (!wsPtrs[i]) continue;
							if (!h) { h = ProcessMgr.CreateScatterHandle(); if (!h) continue; bc = 0; }
							ProcessMgr.AddScatterReadRequest(h, wsPtrs[i] + Offset::MyWeapons, &wvBuf[i], sizeof(WepVec));
							if (++bc >= WEP_BATCH) {
								ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); h = nullptr; bc = 0;
							}
						}
						if (localWsPtr) {
							if (!h) { h = ProcessMgr.CreateScatterHandle(); }
							if (h) ProcessMgr.AddScatterReadRequest(h, localWsPtr + Offset::MyWeapons, &localWv, sizeof(WepVec));
						}
						if (h) { ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); }
					}

					// Phase W3: read weapon handle arrays (batched)
					DWORD handleArrays[MAX_ENTITIES][MAX_WEAPONS_PER_PLAYER]{};
					DWORD localHandles[MAX_WEAPONS_PER_PLAYER]{};
					{
						VMMDLL_SCATTER_HANDLE h = nullptr;
						int bc = 0;
						for (int i = 0; i < cnt; i++) {
							if (!wvBuf[i].dataPtr || wvBuf[i].count <= 0) continue;
							if (!h) { h = ProcessMgr.CreateScatterHandle(); if (!h) continue; bc = 0; }
							int wc = wvBuf[i].count;
							if (wc > MAX_WEAPONS_PER_PLAYER) wc = MAX_WEAPONS_PER_PLAYER;
							ProcessMgr.AddScatterReadRequest(h, wvBuf[i].dataPtr, handleArrays[i], wc * sizeof(DWORD));
							if (++bc >= WEP_BATCH) {
								ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); h = nullptr; bc = 0;
							}
						}
						if (localWv.dataPtr && localWv.count > 0) {
							if (!h) { h = ProcessMgr.CreateScatterHandle(); }
							if (h) {
								int wc = localWv.count;
								if (wc > MAX_WEAPONS_PER_PLAYER) wc = MAX_WEAPONS_PER_PLAYER;
								ProcessMgr.AddScatterReadRequest(h, localWv.dataPtr, localHandles, wc * sizeof(DWORD));
							}
						}
						if (h) { ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); }
					}

					// Helper: resolve weapon handle → entity address → weapon name
					// Must dereference entityListAddr first, same pattern as entity discovery
					DWORD64 entityListDeref = 0;
					ProcessMgr.ReadMemory<DWORD64>(gGame.GetEntityListAddress(), entityListDeref);
					auto resolveWeaponName = [entityListDeref](DWORD handle) -> std::string {
						if (handle == 0 || handle == 0xFFFFFFFF) return "";
						if (!entityListDeref) return "";
						DWORD idx = handle & 0x7FFF;
						DWORD64 subList = 0;
						if (!ProcessMgr.ReadMemory<DWORD64>(entityListDeref + 0x10 + 8 * (idx >> 9), subList) || !subList)
							return "";
						DWORD64 weaponAddr = 0;
						if (!ProcessMgr.ReadMemory<DWORD64>(subList + 0x70 * (idx & 0x1FF), weaponAddr) || !weaponAddr)
							return "";
						DWORD64 nameAddr = ProcessMgr.TraceAddress(weaponAddr + 0x10, { 0x20, 0x0 });
						if (!nameAddr) return "";
						char buf[64]{};
						ProcessMgr.ReadMemory(nameAddr, buf, sizeof(buf) - 1);
						buf[63] = '\0';
						std::string s(buf);
						auto pos = s.find("_");
						if (pos == std::string::npos || s.empty()) return "";
						return s.substr(pos + 1);
					};

					// Phase W4: resolve all weapons for each entity
					for (int i = 0; i < cnt; i++) {
						std::vector<std::string> weapons;
						int wc = wvBuf[i].count;
						if (wc > MAX_WEAPONS_PER_PLAYER) wc = MAX_WEAPONS_PER_PLAYER;
						for (int w = 0; w < wc; w++) {
							std::string name = resolveWeaponName(handleArrays[i][w]);
							if (!name.empty())
								weapons.push_back(name);
						}
						entityCache[i].entity.Pawn.WeaponList = std::move(weapons);
					}
					{
						std::vector<std::string> weapons;
						int wc = localWv.count;
						if (wc > MAX_WEAPONS_PER_PLAYER) wc = MAX_WEAPONS_PER_PLAYER;
						for (int w = 0; w < wc; w++) {
							std::string name = resolveWeaponName(localHandles[w]);
							if (!name.empty())
								weapons.push_back(name);
						}
						localPlayer.Pawn.WeaponList = std::move(weapons);
					}
				}
			}

			} else {
				// Only projectile ESP on: read local player pos for distance calc, skip entity pipeline
				entityCache.clear();
				if (localPlayer.Pawn.Address != 0) {
					Vec3 lpPos{};
					if (ProcessMgr.ReadMemory(localPlayer.Pawn.Address + Offset::Pos, lpPos) && IsValidPos(lpPos))
						localPlayer.Pawn.Pos = lpPos;
				}
			}

			// ------- 8. Grenade projectile scanning (~100ms) -------
			static std::vector<GrenadeProjectile> projectileCache;
			static std::set<DWORD64> expiredEntities;
			if (MenuConfig::ShowProjectileESP) {
				static int projCounter = 0;
				if (++projCounter >= 10) { // ~20ms at 2ms cycle
					projCounter = 0;
					std::vector<GrenadeProjectile> newProjectiles;

					// Scan entities across chunk 0 (64-511) and chunk 1 (512-1023)
					DWORD64 entityListPtr = 0;
					ProcessMgr.ReadMemory<DWORD64>(gGame.GetEntityListAddress(), entityListPtr);
					if (entityListPtr != 0) {
						constexpr int SCAN_START = 64;
						constexpr int SCAN_END = 1024;
						constexpr int SCAN_COUNT = SCAN_END - SCAN_START;

						// Read sub-list pointers for chunk 0 and chunk 1
						DWORD64 chunkPtrs[2]{};
						{
							VMMDLL_SCATTER_HANDLE h = ProcessMgr.CreateScatterHandle();
							if (h) {
								ProcessMgr.AddScatterReadRequest(h, entityListPtr + 0x10, &chunkPtrs[0], sizeof(DWORD64));
								ProcessMgr.AddScatterReadRequest(h, entityListPtr + 0x18, &chunkPtrs[1], sizeof(DWORD64));
								ProcessMgr.ExecuteReadScatter(h);
								VMMDLL_Scatter_CloseHandle(h);
							}
						}

						// Phase 1: Scatter-read all entity addresses (using correct chunk + slot)
						// Contiguous array reads: 960 entries × 0x70 stride ≈ 17 pages across 2 chunks.
						// Batch 200 entries ≈ 6 pages to stay within VMMDLL limit.
						DWORD64 entAddrs[SCAN_COUNT]{};
						{
							constexpr int PROJ_ADDR_BATCH = 200;
							for (int batchStart = 0; batchStart < SCAN_COUNT; batchStart += PROJ_ADDR_BATCH) {
								int batchEnd = (batchStart + PROJ_ADDR_BATCH < SCAN_COUNT) ? batchStart + PROJ_ADDR_BATCH : SCAN_COUNT;
								VMMDLL_SCATTER_HANDLE h = ProcessMgr.CreateScatterHandle();
								if (!h) continue;
								for (int i = batchStart; i < batchEnd; i++) {
									int idx = SCAN_START + i;
									int chunk = idx / 512;
									int slot = idx % 512;
									if (chunk < 2 && chunkPtrs[chunk] != 0)
										ProcessMgr.AddScatterReadRequest(h, chunkPtrs[chunk] + (DWORD64)slot * 0x70, &entAddrs[i], sizeof(DWORD64));
								}
								ProcessMgr.ExecuteReadScatter(h);
								VMMDLL_Scatter_CloseHandle(h);
							}
						}

						// Phases 2-4: random address reads, batch by actual non-zero count.
						// Each valid entry is at a different heap address = 1 unique page each.
						constexpr int PROJ_RAND_BATCH = 6;

						// Phase 2: Scatter-read identity pointers for valid entities
						DWORD64 identityPtrs[SCAN_COUNT]{};
						{
							VMMDLL_SCATTER_HANDLE h = nullptr;
							int bc = 0;
							for (int i = 0; i < SCAN_COUNT; i++) {
								if (entAddrs[i] == 0) continue;
								if (!h) { h = ProcessMgr.CreateScatterHandle(); if (!h) continue; bc = 0; }
								ProcessMgr.AddScatterReadRequest(h, entAddrs[i] + Offset::EntityIdentity, &identityPtrs[i], sizeof(DWORD64));
								if (++bc >= PROJ_RAND_BATCH) {
									ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); h = nullptr; bc = 0;
								}
							}
							if (h) { ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); }
						}

						// Phase 3: Scatter-read designer name pointers
						DWORD64 namePtrs[SCAN_COUNT]{};
						{
							VMMDLL_SCATTER_HANDLE h = nullptr;
							int bc = 0;
							for (int i = 0; i < SCAN_COUNT; i++) {
								if (identityPtrs[i] == 0) continue;
								if (!h) { h = ProcessMgr.CreateScatterHandle(); if (!h) continue; bc = 0; }
								ProcessMgr.AddScatterReadRequest(h, identityPtrs[i] + Offset::DesignerName, &namePtrs[i], sizeof(DWORD64));
								if (++bc >= PROJ_RAND_BATCH) {
									ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); h = nullptr; bc = 0;
								}
							}
							if (h) { ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); }
						}

						// Phase 4: Scatter-read designer name strings
						char nameStrings[SCAN_COUNT][40]{};
						{
							VMMDLL_SCATTER_HANDLE h = nullptr;
							int bc = 0;
							for (int i = 0; i < SCAN_COUNT; i++) {
								if (namePtrs[i] == 0) continue;
								if (!h) { h = ProcessMgr.CreateScatterHandle(); if (!h) continue; bc = 0; }
								ProcessMgr.AddScatterReadRequest(h, namePtrs[i], nameStrings[i], 39);
								if (++bc >= PROJ_RAND_BATCH) {
									ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); h = nullptr; bc = 0;
								}
							}
							if (h) { ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); }
						}

					// Phase 5: Identify grenade projectiles
						struct ProjCandidate { int idx; GrenadeProjectileType type; float radius; };
						std::vector<ProjCandidate> candidates;
						for (int i = 0; i < SCAN_COUNT; i++) {
							nameStrings[i][39] = '\0';
							if (nameStrings[i][0] == '\0') continue;
							const char* n = nameStrings[i];
							GrenadeProjectileType type = PROJ_UNKNOWN;
							float radius = 0;
							if (strstr(n, "smokegrenade_projectile"))        { type = PROJ_SMOKE;   radius = 0.f; }
							else if (strstr(n, "flashbang_projectile"))       { type = PROJ_FLASH;   radius = 0.f; }
							else if (strstr(n, "hegrenade_projectile"))       { type = PROJ_HE;      radius = 350.f; }
							else if (strstr(n, "molotov_projectile") || strstr(n, "incendiarygrenade_proj")) { type = PROJ_MOLOTOV; radius = 150.f; }
							else if (strstr(n, "decoy_projectile"))           { type = PROJ_DECOY;   radius = 0.f; }
							if (type == PROJ_UNKNOWN) continue;
							candidates.push_back({ i, type, radius });
						}

						if (!candidates.empty()) {
							// Phase 6: Scatter-read GameSceneNode for candidates (batched)
							DWORD64 sceneNodes[SCAN_COUNT]{};
							{
								VMMDLL_SCATTER_HANDLE h = nullptr;
								int bc = 0;
								for (auto& c : candidates) {
									if (!h) { h = ProcessMgr.CreateScatterHandle(); if (!h) continue; bc = 0; }
									ProcessMgr.AddScatterReadRequest(h, entAddrs[c.idx] + Offset::GameSceneNode, &sceneNodes[c.idx], sizeof(DWORD64));
									if (++bc >= PROJ_RAND_BATCH) {
										ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); h = nullptr; bc = 0;
									}
								}
								if (h) { ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); }
							}

							// Phase 7: Scatter-read positions (batched)
							Vec3 positions[SCAN_COUNT]{};
							{
								VMMDLL_SCATTER_HANDLE h = nullptr;
								int bc = 0;
								for (auto& c : candidates) {
									if (sceneNodes[c.idx] == 0) continue;
									if (!h) { h = ProcessMgr.CreateScatterHandle(); if (!h) continue; bc = 0; }
									ProcessMgr.AddScatterReadRequest(h, sceneNodes[c.idx] + Offset::vecAbsOrigin, &positions[c.idx], sizeof(Vec3));
									if (++bc >= PROJ_RAND_BATCH) {
										ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); h = nullptr; bc = 0;
									}
								}
								if (h) { ProcessMgr.ExecuteReadScatter(h); VMMDLL_Scatter_CloseHandle(h); }
							}

							// Skip expired entities (e.g. smoke that finished but entity lingers)

							for (auto& c : candidates) {
								DWORD64 addr = entAddrs[c.idx];
								if (expiredEntities.count(addr)) continue;
								Vec3& pos = positions[c.idx];
								if (!std::isfinite(pos.x) || !std::isfinite(pos.y) || !std::isfinite(pos.z)) continue;
								if (std::abs(pos.x) < 1.f && std::abs(pos.y) < 1.f && std::abs(pos.z) < 1.f) continue;
								GrenadeProjectile proj;
								proj.Position = pos;
								proj.EffectRadius = c.radius;
								proj.Type = c.type;
								proj.EntityAddr = addr;
								proj.Alive = true;
								proj.DisappearTimer = 0.f;
								newProjectiles.push_back(proj);
							}

							// Clean up expired set: remove entries whose entities left the list
							std::set<DWORD64> stillInList;
							for (auto& c : candidates) stillInList.insert(entAddrs[c.idx]);
							for (auto it = expiredEntities.begin(); it != expiredEntities.end(); ) {
								if (!stillInList.count(*it)) it = expiredEntities.erase(it);
								else ++it;
							}
						}
					}

					// Merge with previous cache
					static auto lastScanTime = std::chrono::steady_clock::now();
					auto now = std::chrono::steady_clock::now();
					float dt = std::chrono::duration<float>(now - lastScanTime).count();
					lastScanTime = now;

					// Build map of alive entities for quick lookup
					std::unordered_map<DWORD64, int> aliveMap;
					for (int i = 0; i < (int)newProjectiles.size(); i++)
						aliveMap[newProjectiles[i].EntityAddr] = i;

					// Carry over state from previous cache
					for (auto& old : projectileCache) {
						auto it = aliveMap.find(old.EntityAddr);
						if (it != aliveMap.end()) {
							// Entity still alive: carry over StationaryTimer
							auto& np = newProjectiles[it->second];
							float dx = np.Position.x - old.Position.x;
							float dy = np.Position.y - old.Position.y;
							float dz = np.Position.z - old.Position.z;
							if (dx * dx + dy * dy + dz * dz < 1.0f)
								np.StationaryTimer = old.StationaryTimer + dt;
							else
								np.StationaryTimer = 0.f;
						} else {
							// Entity disappeared: handle linger
							if (!old.Alive) {
								old.DisappearTimer += dt;
							} else {
								old.Alive = false;
								old.DisappearTimer = 0.f;
							}
							// Linger: molotov 7s, smoke uses remaining duration
							float maxLinger = 0.f;
							if (old.Type == PROJ_MOLOTOV) maxLinger = 7.0f;
							else if (old.Type == PROJ_SMOKE) maxLinger = std::max(0.f, 18.0f - old.StationaryTimer);
							if (old.DisappearTimer < maxLinger) {
								newProjectiles.push_back(old);
							}
						}
					}

					// Expire projectiles that exceeded their stationary duration
					// HE: immediate on stop, Smoke: 18s
					for (auto& p : newProjectiles) {
						if (!p.Alive) continue;
						bool expired = false;
						if (p.Type == PROJ_HE && p.StationaryTimer > 0.3f) expired = true;
						if (p.Type == PROJ_SMOKE && p.StationaryTimer > 18.0f) expired = true;
						if (expired) expiredEntities.insert(p.EntityAddr);
					}
					newProjectiles.erase(
						std::remove_if(newProjectiles.begin(), newProjectiles.end(),
							[](const GrenadeProjectile& p) {
								if (!p.Alive) return false;
								if (p.Type == PROJ_HE && p.StationaryTimer > 0.3f) return true;
								if (p.Type == PROJ_SMOKE && p.StationaryTimer > 18.0f) return true;
								return false;
							}),
						newProjectiles.end());

					projectileCache = std::move(newProjectiles);
				}
			} else {
				projectileCache.clear();
				expiredEntities.clear();
			}

			// ------- 9. Build entity list and publish snapshot -------
			{
				std::vector<CEntity> publishEntities;
				publishEntities.reserve(entityCache.size());
				for (const auto& ce : entityCache) {
					publishEntities.push_back(ce.entity);
				}

				std::unique_lock<std::shared_mutex> lock(Cheats::SnapshotMutex);
				memcpy(Cheats::Snapshot.Matrix, matrix, sizeof(matrix));
				Cheats::Snapshot.LocalPlayer = localPlayer;
				Cheats::Snapshot.LocalPlayer.LocalPlayerControllerIndex = localPlayerIndex;
				Cheats::Snapshot.Entities.swap(publishEntities);
				Cheats::Snapshot.Projectiles = projectileCache;
			}
		}
		catch (...) {
			// Swallow — last_good_snapshot is preserved
		}
	}
}

// =====================================================================
//  SlowUpdateThread — low-frequency updates
// =====================================================================

VOID SlowUpdateThread()
{
	while (true)
	{
		try {
			if (globalVars::gameState.load() != AppState::RUNNING) {
				Sleep(1000);
				continue;
			}
			gGame.UpdateEntityListEntry();

			uintptr_t mapaddress = 0;
			uintptr_t mapaddress2 = 0;

			if (!ProcessMgr.ReadMemory(gGame.GetClientDLLAddress() + Offset::GlobalVars, mapaddress)) {
				Sleep(5000);
				continue;
			}

			if (!ProcessMgr.ReadMemory(mapaddress + Offset::GlobalVar.CurrentMap, mapaddress2)) {
				Sleep(5000);
				continue;
			}

			char tempMap[32]{};
			ProcessMgr.ReadMemory(mapaddress2, tempMap, 32);

			{
				std::unique_lock<std::shared_mutex> lock(Cheats::SnapshotMutex);
				memcpy(Cheats::Snapshot.MapName, tempMap, sizeof(tempMap));
			}

			Sleep(10000);
		}
		catch (...) {
			Sleep(5000);
		}
	}
}

// =====================================================================
//  KeysCheckThread — keyboard polling (unchanged logic)
// =====================================================================

VOID KeysCheckThread()
{
	while (true)
	{
		Sleep(10);
		Keys::MenuKey = ProcessMgr.is_key_down(VK_F8);

		bool recordKeyPressed = ProcessMgr.is_key_down(GrenadeHelper::RecordHotKey);

		if (recordKeyPressed && !Keys::RecordKey) {
			CEntity localCopy;
			bool valid = false;
			{
				std::shared_lock<std::shared_mutex> lock(Cheats::SnapshotMutex);
				const auto& lp = Cheats::Snapshot.LocalPlayer;
				if (lp.Controller.Address != 0 &&
				    lp.Pawn.Address != 0 &&
				    lp.Pawn.Health > 0) {
					localCopy = lp;
					valid = true;
				}
			}
			if (valid)
				GrenadeHelper::RecordPosition(localCopy);
		}
		Keys::RecordKey = recordKeyPressed;
	}
}
