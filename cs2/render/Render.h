#pragma once

#define _USE_MATH_DEFINES

#include <math.h>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <unordered_map>

#include "../game/Entity.h"
#include "Cheats.h"
#include "../game/MenuConfig.h"

namespace Render
{
	inline bool IsValidFloat(float v) { return std::isfinite(v) && std::abs(v) < 1e7f; }
	inline bool IsValidScreenPos(const ImVec2& p) { return IsValidFloat(p.x) && IsValidFloat(p.y); }
	inline bool IsValidRect(const ImVec4& r) { return IsValidFloat(r.x) && IsValidFloat(r.y) && IsValidFloat(r.z) && IsValidFloat(r.w) && r.z >= 1.f && r.w >= 1.f; }

	void LineToEnemy(ImVec4 Rect, ImColor Color, float Thickness);
	void DrawFov(const CEntity& LocalEntity, float Size, ImColor Color, float Thickness);
	void DrawDistance(const CEntity& LocalEntity, CEntity& Entity, ImVec4 Rect);

	ImVec4 Get2DBox(const CEntity& Entity);



	void DrawBone(const CEntity& Entity, ImColor Color, float Thickness);
	void ShowLosLine(const CEntity& Entity, const float Length, ImColor Color, float Thickness, const float Matrix[4][4]);
	ImVec4 Get2DBoneRect(const CEntity& Entity);

	class HealthBar
	{
	private:
		using TimePoint_ = std::chrono::steady_clock::time_point;
		const int ShowBackUpHealthDuration = 500;
		float MaxHealth = 0.f;
		float CurrentHealth = 0.f;
		float LastestBackupHealth = 0.f;
		ImVec2 RectPos{};
		ImVec2 RectSize{};
		bool InShowBackupHealth = false;
		TimePoint_ BackupHealthTimePoint{};
	public:
		HealthBar() {}
		void DrawHealthBar_Horizontal(float MaxHealth, float CurrentHealth, ImVec2 Pos, ImVec2 Size);
		void DrawHealthBar_Vertical(float MaxHealth, float CurrentHealth, ImVec2 Pos, ImVec2 Size);
	private:
		ImColor Mix(ImColor Col_1, ImColor Col_2, float t);
		ImColor FirstStageColor = ImColor(96, 246, 113, 220);
		ImColor SecondStageColor = ImColor(247, 214, 103, 220);
		ImColor ThirdStageColor = ImColor(255, 95, 95, 220);
		ImColor BackupHealthColor = ImColor(255, 255, 255, 220);
		ImColor FrameColor = ImColor(45, 45, 45, 220);
		ImColor BackGroundColor = ImColor(90, 90, 90, 220);
	};

	void DrawHealthBar(DWORD Sign, float MaxHealth, float CurrentHealth, ImVec2 Pos, ImVec2 Size, bool Horizontal);
	void DrawCornerBox(ImVec4 Rect, ImColor Color, float Thickness, float CornerFrac);
	void DrawHeadDot(const CEntity& Entity, ImColor Color, float Radius);
	void DrawArmorBar(int Armor, ImVec4 Rect, ImColor Color, float BarWidth, int Type);
	void DrawBoxFill(ImVec4 Rect, ImColor Color, float FillAlpha);
	void LineToEnemyEx(ImVec4 Rect, ImColor Color, float Thickness, int Origin);
	void DrawBombESP(const BombData& bomb, const float matrix[4][4]);
	void DrawProjectileESP(const std::vector<GrenadeProjectile>& projectiles, const float matrix[4][4], const Vec3& localPos);
}
