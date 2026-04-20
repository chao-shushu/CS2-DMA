#include "../game/AppState.h"

#include "GUI.h"

#include "Cheats.h"
#include "Render.h"
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <string>
#include "../game/MenuConfig.h"
#include "GrenadeHelper.h"
#include "../config/ConfigMenu.h"
#include "../config/ConfigSaver.h"
#include "../utils/Logger.h"



void Cheats::Run()
{
	static bool firstRun = true;
	if (firstRun) {
		LOG_INFO("Render", "Cheats::Run() first initialization");
		firstRun = false;
	}
	try {
		static std::chrono::time_point LastTimePoint = std::chrono::steady_clock::now();
		auto CurTimePoint = std::chrono::steady_clock::now();

		if (Keys::MenuKey
			&& CurTimePoint - LastTimePoint >= std::chrono::milliseconds(150))
		{
			MenuConfig::ShowMenu = !MenuConfig::ShowMenu;
			LastTimePoint = CurTimePoint;
		}

		if (MenuConfig::ShowMenu)
			Menu();

		AppState currentState = globalVars::gameState.load();
		if (currentState != AppState::RUNNING)
		{
			ImDrawList* drawList = ImGui::GetBackgroundDrawList();
			ImVec2 screenSize = ImGui::GetIO().DisplaySize;

			const char* statusText = "";
			switch (currentState) {
			case AppState::DMA_INITIALIZING: statusText = lang.status_dma_init.c_str(); break;
			case AppState::DMA_FAILED:       statusText = lang.status_dma_failed.c_str(); break;
			case AppState::SEARCHING_GAME:   statusText = lang.status_searching.c_str(); break;
			case AppState::INITIALIZING_GAME:statusText = lang.status_init_game.c_str(); break;
			default:                         statusText = lang.status_unknown.c_str(); break;
			}

			ImVec2 textSize = ImGui::CalcTextSize(statusText);
			ImVec2 textPos = { (screenSize.x - textSize.x) / 2, (screenSize.y - textSize.y) / 2 };

			drawList->AddRectFilled(
				{ textPos.x - 20, textPos.y - 10 },
				{ textPos.x + textSize.x + 20, textPos.y + textSize.y + 10 },
				IM_COL32(0, 0, 0, 180), 8.0f);
			drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), statusText);

			return;
		}

		// Single lock to snapshot all shared state
		GameSnapshot snap;
		{
			std::shared_lock<std::shared_mutex> lock(Cheats::SnapshotMutex);
			snap = Cheats::Snapshot;
		}
		LOG_TRACE("Render", "Snapshot: entities={} projs={} map='{}'", snap.Entities.size(), snap.Projectiles.size(), snap.MapName);

		const auto& LocalPlayerSnapshot = snap.LocalPlayer;

		// Read fresh ViewMatrix every render frame (single 64-byte DMA read).
		// Entity world positions update at DMA rate (~100-200Hz), but view angle
		// changes every game frame. Re-projecting here ensures ESP tracks view
		// rotation at display refresh rate — eliminates stutter during mouse movement.
		float freshMatrix[4][4];
		if (!ProcessMgr.ReadMemory(gGame.GetMatrixAddress(), freshMatrix, 64)) {
			LOG_TRACE("Render", "Fresh matrix read FAILED, using snapshot matrix");
			memcpy(freshMatrix, snap.Matrix, sizeof(freshMatrix));
		}

		// Re-project all entities: world coords → screen coords with fresh matrix
		for (auto& Entity : snap.Entities) {
			if (Entity.Pawn.Health <= 0 || !Entity.Pawn.ScreenPosValid) continue;

			Vec2 footScreen;
			Entity.Pawn.ScreenPosValid = CView::WorldToScreen(freshMatrix, Entity.Pawn.Pos, footScreen);
			Entity.Pawn.ScreenPos = footScreen;

			for (int j = 0; j < Entity.Pawn.BoneData.BonePosCount; j++) {
				auto& bp = Entity.Pawn.BoneData.BonePosList[j];
				if (!bp.IsVisible) continue; // already marked invalid by data thread
				Vec2 sp;
				bp.IsVisible = CView::WorldToScreen(freshMatrix, bp.Pos, sp);
				bp.ScreenPos = sp;
			}
		}

		const auto& EntityListSnapshot = snap.Entities;

		for (int i = 0; i < EntityListSnapshot.size(); i++)
		{
			CEntity Entity = EntityListSnapshot[i];

			if (Entity.Pawn.Health <= 0 || Entity.Pawn.Health > 100) continue;

			if (MenuConfig::TeamCheck && Entity.Controller.TeamID == LocalPlayerSnapshot.Controller.TeamID)
				continue;

			if (!Entity.Pawn.ScreenPosValid) continue;
			if (!std::isfinite(Entity.Pawn.ScreenPos.x) || !std::isfinite(Entity.Pawn.ScreenPos.y)) continue;
			if (!std::isfinite(Entity.Pawn.Pos.x) || !std::isfinite(Entity.Pawn.Pos.y) || !std::isfinite(Entity.Pawn.Pos.z)) continue;
			if (Entity.Controller.Address == 0) continue;
			if (Entity.GetBone().BonePosCount <= (int)BONEINDEX::head) continue;

			if (MenuConfig::ShowBoneESP)
				Render::DrawBone(Entity, MenuConfig::BoneColor, MenuConfig::BoneThickness);

			if (MenuConfig::ShowHeadDot)
				Render::DrawHeadDot(Entity, MenuConfig::HeadDotColor, MenuConfig::HeadDotSize);

			if (MenuConfig::ShowEyeRay)
				Render::ShowLosLine(Entity, MenuConfig::EyeRayLength, MenuConfig::EyeRayColor, MenuConfig::EyeRayThickness, freshMatrix);

			ImVec4 Rect;
			switch (MenuConfig::BoxType)
			{
			case 0:
				Rect = Render::Get2DBox(Entity);
				break;
			case 1:
				Rect = Render::Get2DBoneRect(Entity);
				break;
			case 2:
				Rect = Render::Get2DBox(Entity);
				break;
			default:
				Rect = Render::Get2DBox(Entity);
				break;
			}

			// Skip if box computation returned invalid rect
			if (Rect.z < 1.f || Rect.w < 1.f) continue;
			if (!std::isfinite(Rect.x) || !std::isfinite(Rect.y) || !std::isfinite(Rect.z) || !std::isfinite(Rect.w)) continue;

			if (MenuConfig::ShowLineToEnemy)
				Render::LineToEnemyEx(Rect, MenuConfig::LineToEnemyColor, MenuConfig::LineToEnemyThickness, MenuConfig::LineToEnemyOrigin);

			if (MenuConfig::ShowBoxESP)
			{
				if (MenuConfig::BoxFilled)
					Render::DrawBoxFill(Rect, MenuConfig::BoxColor, MenuConfig::BoxFillAlpha);

				if (MenuConfig::BoxType == 2)
					Render::DrawCornerBox(Rect, MenuConfig::BoxColor, MenuConfig::BoxThickness, MenuConfig::CornerLength);
				else
					Gui.Rectangle({ Rect.x,Rect.y }, { Rect.z,Rect.w }, MenuConfig::BoxColor, MenuConfig::BoxThickness, MenuConfig::BoxRounding);
			}

			if (MenuConfig::ShowHealthBar)
			{
				if (MenuConfig::HealthBarType == 2)
				{
					float hpRatio = std::clamp((float)Entity.Pawn.Health / 100.f, 0.f, 1.f);
					ImColor hpColor = ImColor(
						(int)(255 * (1.f - hpRatio)),
						(int)(255 * hpRatio),
						0, 255);
					float hpY = Rect.y - MenuConfig::NameFontSize;
					if (MenuConfig::ShowPlayerName)
						hpY -= MenuConfig::NameFontSize;
					Gui.StrokeText(std::to_string(Entity.Pawn.Health), Vec2(Rect.x + Rect.z / 2, hpY), hpColor, MenuConfig::NameFontSize, true);
				}
				else
				{
					ImVec2 HealthBarPos, HealthBarSize;
					if (MenuConfig::HealthBarType == 0)
					{
						HealthBarPos = { Rect.x - MenuConfig::HealthBarWidth - 3.f, Rect.y };
						HealthBarSize = { MenuConfig::HealthBarWidth, Rect.w };
					}
					else
					{
						HealthBarPos = { Rect.x + Rect.z / 2 - 70 / 2, Rect.y - 13 };
						HealthBarSize = { 70, 8 };
					}
					Render::DrawHealthBar(Entity.Controller.Address, 100, Entity.Pawn.Health, HealthBarPos, HealthBarSize, MenuConfig::HealthBarType);
				}
			}

			if (MenuConfig::ShowArmorBar)
				Render::DrawArmorBar(Entity.Pawn.Armor, Rect, MenuConfig::ArmorBarColor, MenuConfig::ArmorBarWidth, MenuConfig::ArmorBarType);

			if (MenuConfig::ShowWeaponESP)
				Gui.StrokeText(Entity.Pawn.WeaponName, Vec2(Rect.x, Rect.y + Rect.w), MenuConfig::WeaponColor, MenuConfig::WeaponFontSize);

			if (MenuConfig::ShowDistance)
				Render::DrawDistance(LocalPlayerSnapshot, Entity, Rect);

			if (MenuConfig::ShowPlayerName)
			{
				if (MenuConfig::HealthBarType == 1)
					Gui.StrokeText(Entity.Controller.PlayerName, Vec2(Rect.x + Rect.z / 2, Rect.y - 13 - MenuConfig::NameFontSize), MenuConfig::NameColor, MenuConfig::NameFontSize, true);
				else
					Gui.StrokeText(Entity.Controller.PlayerName, Vec2(Rect.x + Rect.z / 2, Rect.y - MenuConfig::NameFontSize), MenuConfig::NameColor, MenuConfig::NameFontSize, true);
			}
		}

		// Bomb ESP
		if (MenuConfig::ShowBombESP)
			Render::DrawBombESP(snap.Bomb, freshMatrix);

		// Grenade Projectile ESP
		if (MenuConfig::ShowProjectileESP && !snap.Projectiles.empty())
			Render::DrawProjectileESP(snap.Projectiles, freshMatrix, LocalPlayerSnapshot.Pawn.Pos);

		// Crosshair overlay: drawn on top of ESP, below safe zone mask
		if (MenuConfig::CrosshairEnabled) {
			ImVec2 c = { ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f };
			ImDrawList* dl = ImGui::GetBackgroundDrawList();

			// Determine if crosshair is on an enemy
			bool onEnemy = false;
			if (MenuConfig::CrosshairOnEnemyColor) {
				for (const auto& Entity : EntityListSnapshot) {
					if (Entity.Pawn.Health <= 0 || Entity.Pawn.Health > 100) continue;
					if (MenuConfig::TeamCheck && Entity.Controller.TeamID == LocalPlayerSnapshot.Controller.TeamID) continue;
					if (!Entity.Pawn.ScreenPosValid) continue;

					ImVec4 eRect;
					switch (MenuConfig::BoxType) {
					case 1:  eRect = Render::Get2DBoneRect(Entity); break;
					default: eRect = Render::Get2DBox(Entity); break;
					}
					if (eRect.z < 1.f || eRect.w < 1.f) continue;
					if (c.x >= eRect.x && c.x <= eRect.x + eRect.z &&
						c.y >= eRect.y && c.y <= eRect.y + eRect.w) {
						onEnemy = true;
						break;
					}
				}
			}

			ImU32 col = onEnemy
				? ImGui::ColorConvertFloat4ToU32(MenuConfig::CrosshairEnemyColor.Value)
				: ImGui::ColorConvertFloat4ToU32(MenuConfig::CrosshairColor.Value);
			float sz = MenuConfig::CrosshairSize;
			float th = MenuConfig::CrosshairThickness;
			float gap = MenuConfig::CrosshairGap;

			// Style 0: Cross, 1: Dot, 2: Circle, 3: Cross+Dot
			bool drawCross = (MenuConfig::CrosshairStyle == 0 || MenuConfig::CrosshairStyle == 3);
			bool drawDot   = (MenuConfig::CrosshairStyle == 1 || MenuConfig::CrosshairStyle == 3);
			bool drawCircle = (MenuConfig::CrosshairStyle == 2);

			if (drawCross) {
				dl->AddLine({ c.x - gap - sz, c.y }, { c.x - gap, c.y }, col, th);
				dl->AddLine({ c.x + gap, c.y }, { c.x + gap + sz, c.y }, col, th);
				dl->AddLine({ c.x, c.y - gap - sz }, { c.x, c.y - gap }, col, th);
				dl->AddLine({ c.x, c.y + gap }, { c.x, c.y + gap + sz }, col, th);
			}
			if (drawDot) {
				dl->AddCircleFilled({ c.x, c.y }, th, col, 12);
			}
			if (drawCircle) {
				dl->AddCircle({ c.x, c.y }, sz, col, 32, th);
			}
		}

		// Safe zone black mask: drawn on top of all ESP to erase the crosshair area
		if (MenuConfig::SafeZoneEnabled) {
			ImVec2 c = { ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f };
			float r = MenuConfig::SafeZoneRadius;
			ImDrawList* dl = ImGui::GetBackgroundDrawList();
			if (MenuConfig::SafeZoneShape == 0)
				dl->AddCircleFilled({ c.x, c.y }, r, IM_COL32(0, 0, 0, 255), 64);
			else
				dl->AddRectFilled({ c.x - r, c.y - r }, { c.x + r, c.y + r }, IM_COL32(0, 0, 0, 255));
		}

		// Grenade Helper - Update map and render
		static std::string lastMapName;
		if (snap.MapName[0] != '\0' && snap.MapName != lastMapName) {
			LOG_DEBUG("Render", "Map changed: '{}' -> '{}'", lastMapName, snap.MapName);
			GrenadeHelper::UpdateMap(snap.MapName);
			lastMapName = snap.MapName;
		}
		GrenadeHelper::Render(LocalPlayerSnapshot);

		// Handle auto-save for grenade helper
		if (GrenadeHelper::NeedSave) {
			GrenadeHelper::SaveToFile(GrenadeHelper::CurrentMap);
			GrenadeHelper::NeedSave = false;
		}

		// Spectator List overlay
		if (MenuConfig::ShowSpectatorList && !snap.Spectators.empty()) {
			ImDrawList* dl = ImGui::GetBackgroundDrawList();
			ImVec2 screenSize = ImGui::GetIO().DisplaySize;

			float posX = screenSize.x - 220.f;
			float posY = 10.f;
			float lineH = 18.f;
			float padX = 10.f;
			float padY = 6.f;

			// Background
			float bgHeight = padY * 2 + lineH * (1 + (float)snap.Spectators.size());
			dl->AddRectFilled({ posX - padX, posY - padY }, { screenSize.x - 6.f, posY - padY + bgHeight },
				IM_COL32(10, 10, 15, 200), 6.f);
			dl->AddRect({ posX - padX, posY - padY }, { screenSize.x - 6.f, posY - padY + bgHeight },
				ImColor(0.486f, 0.361f, 0.988f, 0.60f), 6.f, 15, 1.f);

			// Title
			dl->AddText({ posX, posY }, ImColor(0.608f, 0.490f, 1.0f, 1.0f), "Spectators");
			posY += lineH;

			for (const auto& spec : snap.Spectators) {
				ImColor nameCol = (spec.TeamID == LocalPlayerSnapshot.Controller.TeamID)
					? ImColor(100, 200, 255, 255)   // teammate: blue
					: ImColor(255, 100, 100, 255);   // enemy: red

				// Observer mode label
				const char* modeStr = "";
				if (spec.ObserverMode == 4) modeStr = "[Eye] ";
				else if (spec.ObserverMode == 5) modeStr = "[Chase] ";
				else if (spec.ObserverMode == 6) modeStr = "[Roam] ";

				char text[128];
				snprintf(text, sizeof(text), "%s%s", modeStr, spec.Name.c_str());
				dl->AddText({ posX, posY }, nameCol, text);
				posY += lineH;
			}
		}

		// Performance Monitor overlay
		if (MenuConfig::ShowPerfMonitor) {
			ImDrawList* dl = ImGui::GetBackgroundDrawList();

			float posX = 10.f;
			float posY = 10.f;
			float lineH = 16.f;
			float padX = 10.f;
			float padY = 6.f;

			float fps = ImGui::GetIO().Framerate;
			float frameTimeMs = 1000.f / (fps > 0.f ? fps : 1.f);

			// Count lines
			int lines = 5; // FPS, FrameTime, Entities, Projectiles, Spectators
			float bgHeight = padY * 2 + lineH * lines;

			dl->AddRectFilled({ posX - padX, posY - padY }, { posX + 180.f + padX, posY - padY + bgHeight },
				IM_COL32(10, 10, 15, 200), 6.f);
			dl->AddRect({ posX - padX, posY - padY }, { posX + 180.f + padX, posY - padY + bgHeight },
				ImColor(0.486f, 0.361f, 0.988f, 0.60f), 6.f, 15, 1.f);

			ImColor labelCol(180, 180, 200, 255);
			ImColor valueCol(255, 255, 255, 255);

			// FPS (color coded)
			ImColor fpsCol;
			if (fps >= 60.f) fpsCol = ImColor(0, 220, 100, 255);
			else if (fps >= 30.f) fpsCol = ImColor(255, 200, 0, 255);
			else fpsCol = ImColor(255, 50, 50, 255);

			char buf[64];
			snprintf(buf, sizeof(buf), "%.0f", fps);
			dl->AddText({ posX, posY }, labelCol, "FPS:");
			dl->AddText({ posX + 80.f, posY }, fpsCol, buf);
			posY += lineH;

			snprintf(buf, sizeof(buf), "%.1f ms", frameTimeMs);
			dl->AddText({ posX, posY }, labelCol, "Frame:");
			dl->AddText({ posX + 80.f, posY }, valueCol, buf);
			posY += lineH;

			snprintf(buf, sizeof(buf), "%d", (int)snap.Entities.size());
			dl->AddText({ posX, posY }, labelCol, "Entities:");
			dl->AddText({ posX + 80.f, posY }, valueCol, buf);
			posY += lineH;

			snprintf(buf, sizeof(buf), "%d", (int)snap.Projectiles.size());
			dl->AddText({ posX, posY }, labelCol, "Projectiles:");
			dl->AddText({ posX + 80.f, posY }, valueCol, buf);
			posY += lineH;

			snprintf(buf, sizeof(buf), "%d", (int)snap.Spectators.size());
			dl->AddText({ posX, posY }, labelCol, "Spectators:");
			dl->AddText({ posX + 80.f, posY }, valueCol, buf);
		}

		// Auto-save config every 5 seconds
		{
			static auto lastAutoSave = std::chrono::steady_clock::now();
			auto now = std::chrono::steady_clock::now();
			if (now - lastAutoSave >= std::chrono::seconds(5)) {
				MyConfigSaver::SaveConfig("_autosave.config");
				lastAutoSave = now;
			}
		}

	}
	catch (std::exception const& e)
	{
		LOG_ERROR("Render", "Cheats::Run exception: {}", e.what());
	}
	catch (...)
	{
		LOG_ERROR("Render", "Cheats::Run unknown exception");
	}
}
