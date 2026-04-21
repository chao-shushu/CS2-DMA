#include "Render.h"



namespace Render

{





	void LineToEnemy(ImVec4 Rect, ImColor Color, float Thickness)

	{

		ImVec2 ds = ImGui::GetIO().DisplaySize;
		Gui.Line({ Rect.x + Rect.z / 2,Rect.y }, { ds.x / 2,0 }, Color, Thickness);

	}



	void DrawFov(const CEntity& LocalEntity, float Size, ImColor Color, float Thickness)

	{

		if (!IsValidFloat(Size) || Size <= 0.f) return;
		float Length;

		float radian;

		Vec2 LineEndPoint[2];

		ImVec2 ds2 = ImGui::GetIO().DisplaySize;
		Vec2 Pos = Vec2(ds2.x / 2, ds2.y / 2);



		radian = (LocalEntity.Pawn.Fov / 2) * M_PI / 180;



		LineEndPoint[0].y = Pos.y - Size;

		LineEndPoint[1].y = LineEndPoint[0].y;



		Length = Size * tan(radian);



		LineEndPoint[0].x = Pos.x - Length;

		LineEndPoint[1].x = Pos.x + Length;



		Gui.Line(Pos, LineEndPoint[0], Color, 1.5);

		Gui.Line(Pos, LineEndPoint[1], Color, 1.5);

	}



	void DrawDistance(const CEntity& LocalEntity, const CEntity& Entity, ImVec4 Rect)

	{

		if (!IsValidRect(Rect)) return;
		if (!IsValidFloat(Entity.Pawn.Pos.x) || !IsValidFloat(LocalEntity.Pawn.Pos.x)) return;
		int distance = static_cast<int>(Entity.Pawn.Pos.DistanceTo(LocalEntity.Pawn.Pos) / 100);

		std::string dis_str = std::to_string(distance) + "m";

		Gui.StrokeText(dis_str, Vec2(Rect.x + Rect.z + 4, Rect.y), ImColor(255, 255, 255, 255), 14, false);

	}





	ImVec4 Get2DBox(const CEntity& Entity)

	{

		const CBone& bone = Entity.GetBone();

		// Safety check: ensure bone list has enough elements for head index

		if (bone.BonePosCount <= BONEINDEX::head) {

			return ImVec4{ 0, 0, 0, 0 };

		}



		const BoneJointPos& Head = bone.BonePosList[BONEINDEX::head];

		if (!Head.IsVisible) return ImVec4{ 0, 0, 0, 0 };



		Vec2 Size, Pos;

		Size.y = (Entity.Pawn.ScreenPos.y - Head.ScreenPos.y) * 1.09f;

		Size.x = Size.y * 0.6f;



		// Reject invalid boxes (negative/zero size or extreme dimensions)

		if (Size.y < 4.f || Size.x < 2.f || Size.y > 4000.f) return ImVec4{ 0, 0, 0, 0 };



		Pos = ImVec2(Entity.Pawn.ScreenPos.x - Size.x / 2, Head.ScreenPos.y - Size.y * 0.18f);



		return ImVec4{ Pos.x,Pos.y,Size.x,Size.y };

	}



	void DrawBone(const CEntity& Entity, ImColor Color, float Thickness)

	{

		const CBone& boneRef = Entity.GetBone();
		if (boneRef.BonePosCount <= 0) return;

		BoneJointPos Previous, Current;

		for (auto i : BoneJointList::List)

		{

			Previous.Pos = Vec3(0, 0, 0);

			for (auto Index : i)

			{

				Current = Entity.GetBone().BonePosList[Index];

				if (Previous.Pos == Vec3(0, 0, 0))

				{

					Previous = Current;

					continue;

				}

				if (Previous.IsVisible && Current.IsVisible)

				{

					Gui.Line(Previous.ScreenPos, Current.ScreenPos, Color, Thickness);

				}

				Previous = Current;

			}

		}

	}



	void ShowLosLine(const CEntity& Entity, const float Length, ImColor Color, float Thickness, const float Matrix[4][4])
	{
		if (!IsValidFloat(Length) || Length <= 0.f) return;
		const CBone& losbone = Entity.GetBone();
		if (losbone.BonePosCount <= BONEINDEX::head) return;

		BoneJointPos Head = Entity.GetBone().BonePosList[BONEINDEX::head];
		if (!Head.IsVisible) return;

		Vec2 StartPoint = Head.ScreenPos;

		float LineLength = cos(Entity.Pawn.ViewAngle.x * M_PI / 180) * Length;

		Vec3 Temp;
		Temp.x = Head.Pos.x + cos(Entity.Pawn.ViewAngle.y * M_PI / 180) * LineLength;
		Temp.y = Head.Pos.y + sin(Entity.Pawn.ViewAngle.y * M_PI / 180) * LineLength;
		Temp.z = Head.Pos.z - sin(Entity.Pawn.ViewAngle.x * M_PI / 180) * Length;

		Vec2 EndPoint;
		if (!CView::WorldToScreen(Matrix, Temp, EndPoint))
			return;

		Gui.Line(StartPoint, EndPoint, Color, Thickness);
	}



	ImVec4 Get2DBoneRect(const CEntity& Entity)

	{

		const CBone& bone = Entity.GetBone();

		// Find first visible bone to seed Min/Max

		bool found = false;

		Vec2 Min{}, Max{};

		for (int i = 0; i < CBone::NUM_BONES; i++)

		{

			if (!bone.BonePosList[i].IsVisible) continue;

			if (!found) {

				Min = Max = bone.BonePosList[i].ScreenPos;

				found = true;

				continue;

			}

			Min.x = std::min(bone.BonePosList[i].ScreenPos.x, Min.x);

			Min.y = std::min(bone.BonePosList[i].ScreenPos.y, Min.y);

			Max.x = std::max(bone.BonePosList[i].ScreenPos.x, Max.x);

			Max.y = std::max(bone.BonePosList[i].ScreenPos.y, Max.y);

		}

		if (!found) return ImVec4{ 0, 0, 0, 0 };



		Vec2 Size;

		Size.x = Max.x - Min.x;

		Size.y = Max.y - Min.y;



		// Reject zero/tiny/extreme boxes

		if (Size.y < 4.f || Size.x < 2.f || Size.y > 4000.f) return ImVec4{ 0, 0, 0, 0 };



		return ImVec4(Min.x, Min.y, Size.x, Size.y);

	}






	void HealthBar::DrawHealthBar_Horizontal(float MaxHealth, float CurrentHealth, ImVec2 Pos, ImVec2 Size)

	{

		if (MaxHealth <= 0.f || !IsValidFloat(MaxHealth) || !IsValidFloat(CurrentHealth)) return;
		if (!IsValidScreenPos(Pos) || !IsValidFloat(Size.x) || !IsValidFloat(Size.y)) return;
		if (Size.x < 1.f || Size.y < 1.f) return;
		CurrentHealth = std::clamp(CurrentHealth, 0.f, MaxHealth);

		auto InRange = [&](float value, float min, float max) -> bool

		{

			return value > min && value <= max;

		};



		ImDrawList* DrawList = ImGui::GetBackgroundDrawList();



		this->MaxHealth = MaxHealth;

		this->CurrentHealth = CurrentHealth;

		this->RectPos = Pos;

		this->RectSize = Size;





		float Proportion = CurrentHealth / MaxHealth;



		float Width = RectSize.x * Proportion;



		ImColor Color;





		DrawList->AddRectFilled(RectPos,

			{ RectPos.x + RectSize.x,RectPos.y + RectSize.y },

			BackGroundColor, 5, 15);





		float Color_Lerp_t = pow(Proportion, 2.5);

		if (InRange(Proportion, 0.5, 1))

			Color = Mix(FirstStageColor, SecondStageColor, Color_Lerp_t * 3 - 1);

		else

			Color = Mix(SecondStageColor, ThirdStageColor, Color_Lerp_t * 4);





		if (LastestBackupHealth == 0

			|| LastestBackupHealth < CurrentHealth)

			LastestBackupHealth = CurrentHealth;



		if (LastestBackupHealth != CurrentHealth)

		{

			if (!InShowBackupHealth)

			{

				BackupHealthTimePoint = std::chrono::steady_clock::now();

				InShowBackupHealth = true;

			}



			std::chrono::steady_clock::time_point CurrentTime = std::chrono::steady_clock::now();

			if (CurrentTime - BackupHealthTimePoint > std::chrono::milliseconds(ShowBackUpHealthDuration))

			{



				LastestBackupHealth = CurrentHealth;

				InShowBackupHealth = false;

			}



			if (InShowBackupHealth)

			{

				float BackupHealthWidth = LastestBackupHealth / MaxHealth * RectSize.x;



				float BackupHealthColorAlpha = 1 - 0.95 * (std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - BackupHealthTimePoint).count() / (float)ShowBackUpHealthDuration);

				ImColor BackupHealthColorTemp = BackupHealthColor;

				BackupHealthColorTemp.Value.w = BackupHealthColorAlpha;



				float BackupHealthWidth_Lerp = 1 * (std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - BackupHealthTimePoint).count() / (float)ShowBackUpHealthDuration);

				BackupHealthWidth_Lerp *= (BackupHealthWidth - Width);

				BackupHealthWidth -= BackupHealthWidth_Lerp;



				DrawList->AddRectFilled(RectPos,

					{ RectPos.x + BackupHealthWidth,RectPos.y + RectSize.y },

					BackupHealthColorTemp, 5);

			}

		}





		DrawList->AddRectFilled(RectPos,

			{ RectPos.x + Width,RectPos.y + RectSize.y },

			Color, 5);





		DrawList->AddRect(RectPos,

			{ RectPos.x + RectSize.x,RectPos.y + RectSize.y },

			FrameColor, 5, 15, 1);

	}



	void HealthBar::DrawHealthBar_Vertical(float MaxHealth, float CurrentHealth, ImVec2 Pos, ImVec2 Size)

	{

		if (MaxHealth <= 0.f || !IsValidFloat(MaxHealth) || !IsValidFloat(CurrentHealth)) return;
		if (!IsValidScreenPos(Pos) || !IsValidFloat(Size.x) || !IsValidFloat(Size.y)) return;
		if (Size.x < 1.f || Size.y < 1.f) return;
		CurrentHealth = std::clamp(CurrentHealth, 0.f, MaxHealth);

		auto InRange = [&](float value, float min, float max) -> bool

		{

			return value > min && value <= max;

		};



		ImDrawList* DrawList = ImGui::GetBackgroundDrawList();



		this->MaxHealth = MaxHealth;

		this->CurrentHealth = CurrentHealth;

		this->RectPos = Pos;

		this->RectSize = Size;





		float Proportion = CurrentHealth / MaxHealth;



		float Height = RectSize.y * Proportion;



		ImColor Color;



		DrawList->AddRectFilled(RectPos,

			{ RectPos.x + RectSize.x,RectPos.y + RectSize.y },

			BackGroundColor, 5, 15);



		float Color_Lerp_t = pow(Proportion, 2.5);

		if (InRange(Proportion, 0.5, 1))

			Color = Mix(FirstStageColor, SecondStageColor, Color_Lerp_t * 3 - 1);

		else

			Color = Mix(SecondStageColor, ThirdStageColor, Color_Lerp_t * 4);



		if (LastestBackupHealth == 0

			|| LastestBackupHealth < CurrentHealth)

			LastestBackupHealth = CurrentHealth;



		if (LastestBackupHealth != CurrentHealth)

		{

			if (!InShowBackupHealth)

			{

				BackupHealthTimePoint = std::chrono::steady_clock::now();

				InShowBackupHealth = true;

			}



			std::chrono::steady_clock::time_point CurrentTime = std::chrono::steady_clock::now();

			if (CurrentTime - BackupHealthTimePoint > std::chrono::milliseconds(ShowBackUpHealthDuration))

			{

				LastestBackupHealth = CurrentHealth;

				InShowBackupHealth = false;

			}



			if (InShowBackupHealth)

			{

				float BackupHealthHeight = LastestBackupHealth / MaxHealth * RectSize.y;

				float BackupHealthColorAlpha = 1 - 0.95 * (std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - BackupHealthTimePoint).count() / (float)ShowBackUpHealthDuration);

				ImColor BackupHealthColorTemp = BackupHealthColor;

				BackupHealthColorTemp.Value.w = BackupHealthColorAlpha;

				float BackupHealthHeight_Lerp = 1 * (std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - BackupHealthTimePoint).count() / (float)ShowBackUpHealthDuration);

				BackupHealthHeight_Lerp *= (BackupHealthHeight - Height);

				BackupHealthHeight -= BackupHealthHeight_Lerp;

				DrawList->AddRectFilled({ RectPos.x,RectPos.y + RectSize.y - BackupHealthHeight },

					{ RectPos.x + RectSize.x,RectPos.y + RectSize.y },

					BackupHealthColorTemp, 5);

			}

		}



		DrawList->AddRectFilled({ RectPos.x,RectPos.y + RectSize.y - Height },

			{ RectPos.x + RectSize.x,RectPos.y + RectSize.y },

			Color, 5);



		DrawList->AddRect(RectPos,

			{ RectPos.x + RectSize.x,RectPos.y + RectSize.y },

			FrameColor, 5, 15, 1);

	}



	ImColor HealthBar::Mix(ImColor Col_1, ImColor Col_2, float t)

	{

		ImColor Col;

		Col.Value.x = t * Col_1.Value.x + (1 - t) * Col_2.Value.x;

		Col.Value.y = t * Col_1.Value.y + (1 - t) * Col_2.Value.y;

		Col.Value.z = t * Col_1.Value.z + (1 - t) * Col_2.Value.z;

		Col.Value.w = Col_1.Value.w;

		return Col;

	}



	void DrawHealthBar(DWORD Sign, float MaxHealth, float CurrentHealth, ImVec2 Pos, ImVec2 Size, bool Horizontal)
	{
		if (MaxHealth <= 0.f || CurrentHealth < 0.f || Sign == 0) return;
		if (!IsValidFloat(CurrentHealth) || !IsValidScreenPos(Pos)) return;
		static std::unordered_map<DWORD, HealthBar> HealthBarMap;
		if (!HealthBarMap.count(Sign))
		{
			HealthBarMap.insert({ Sign,HealthBar() });
		}
		if (HealthBarMap.count(Sign))
		{
			if (Horizontal)
				HealthBarMap[Sign].DrawHealthBar_Horizontal(MaxHealth, CurrentHealth, Pos, Size);
			else
				HealthBarMap[Sign].DrawHealthBar_Vertical(MaxHealth, CurrentHealth, Pos, Size);
		}
	}

	// ================================================================
	//  Corner-style box (only corners drawn, not full edges)
	// ================================================================
	void DrawCornerBox(ImVec4 Rect, ImColor Color, float Thickness, float CornerFrac)
	{
		if (!IsValidRect(Rect)) return;
		ImDrawList* dl = ImGui::GetBackgroundDrawList();
		float x = Rect.x, y = Rect.y, w = Rect.z, h = Rect.w;
		float lx = w * CornerFrac, ly = h * CornerFrac;

		// top-left
		dl->AddLine({ x, y }, { x + lx, y }, Color, Thickness);
		dl->AddLine({ x, y }, { x, y + ly }, Color, Thickness);
		// top-right
		dl->AddLine({ x + w, y }, { x + w - lx, y }, Color, Thickness);
		dl->AddLine({ x + w, y }, { x + w, y + ly }, Color, Thickness);
		// bottom-left
		dl->AddLine({ x, y + h }, { x + lx, y + h }, Color, Thickness);
		dl->AddLine({ x, y + h }, { x, y + h - ly }, Color, Thickness);
		// bottom-right
		dl->AddLine({ x + w, y + h }, { x + w - lx, y + h }, Color, Thickness);
		dl->AddLine({ x + w, y + h }, { x + w, y + h - ly }, Color, Thickness);
	}

	// ================================================================
	//  Head dot (circle on head bone position)
	// ================================================================
	void DrawHeadDot(const CEntity& Entity, ImColor Color, float Radius)
	{
		const CBone& bone = Entity.GetBone();
		if (bone.BonePosCount <= BONEINDEX::head) return;
		if (!bone.BonePosList[BONEINDEX::head].IsVisible) return;
		Vec2 headScreen = bone.BonePosList[BONEINDEX::head].ScreenPos;
		if (!IsValidFloat(headScreen.x) || !IsValidFloat(headScreen.y)) return;

		ImDrawList* dl = ImGui::GetBackgroundDrawList();
		dl->AddCircleFilled({ headScreen.x, headScreen.y }, Radius, Color, 12);
		dl->AddCircle({ headScreen.x, headScreen.y }, Radius, ImColor(0, 0, 0, 200), 12, 1.f);
	}

	// ================================================================
	//  Armor bar (simple proportional bar)
	// ================================================================
	void DrawArmorBar(int Armor, ImVec4 Rect, ImColor Color, float BarWidth, int Type)
	{
		if (Armor <= 0 || Armor > 100) return;
		if (!IsValidRect(Rect)) return;
		ImDrawList* dl = ImGui::GetBackgroundDrawList();
		float proportion = Armor / 100.f;
		ImColor bgCol(90, 90, 90, 160);
		ImColor frameCol(45, 45, 45, 180);

		if (Type == 0) {
			// vertical, right side of box
			float x = Rect.x + Rect.z + 2.f;
			float y = Rect.y;
			float h = Rect.w;
			dl->AddRectFilled({ x, y }, { x + BarWidth, y + h }, bgCol, 2);
			dl->AddRectFilled({ x, y + h * (1.f - proportion) }, { x + BarWidth, y + h }, Color, 2);
			dl->AddRect({ x, y }, { x + BarWidth, y + h }, frameCol, 2, 0, 1);
		}
		else {
			// horizontal, above box
			float x = Rect.x;
			float y = Rect.y - BarWidth - 2.f;
			float w = Rect.z;
			dl->AddRectFilled({ x, y }, { x + w, y + BarWidth }, bgCol, 2);
			dl->AddRectFilled({ x, y }, { x + w * proportion, y + BarWidth }, Color, 2);
			dl->AddRect({ x, y }, { x + w, y + BarWidth }, frameCol, 2, 0, 1);
		}
	}

	// ================================================================
	//  Box fill background
	// ================================================================
	void DrawBoxFill(ImVec4 Rect, ImColor Color, float FillAlpha)
	{
		if (!IsValidRect(Rect)) return;
		ImDrawList* dl = ImGui::GetBackgroundDrawList();
		ImColor c = Color;
		c.Value.w = FillAlpha;
		dl->AddRectFilled({ Rect.x, Rect.y }, { Rect.x + Rect.z, Rect.y + Rect.w }, c);
	}

	// ================================================================
	//  Snapline with configurable origin point
	// ================================================================
	void LineToEnemyEx(ImVec4 Rect, ImColor Color, float Thickness, int Origin)
	{
		if (!IsValidRect(Rect)) return;
		Vec2 entityTop;
		entityTop.x = Rect.x + Rect.z / 2;
		entityTop.y = Rect.y;
		Vec2 from;
		ImVec2 ds3 = ImGui::GetIO().DisplaySize;
		switch (Origin) {
		default:
		case 0: from.x = ds3.x / 2; from.y = 0; break;
		case 1: from.x = ds3.x / 2; from.y = ds3.y / 2; break;
		case 2: from.x = ds3.x / 2; from.y = ds3.y; break;
		}
		Gui.Line(entityTop, from, Color, Thickness);
	}

	// ================================================================
	//  Bomb ESP: dot + C4 text + timer + defuse info
	// ================================================================
	void DrawBombESP(const BombData& bomb, const float matrix[4][4])
	{
		if (bomb.x == 0 && bomb.y == 0) return;
		if (!IsValidFloat(bomb.x) || !IsValidFloat(bomb.y) || !IsValidFloat(bomb.z)) return;

		Vec3 bombWorld{ bomb.x, bomb.y, bomb.z };
		Vec2 screenPos;
		if (!CView::WorldToScreen(matrix, bombWorld, screenPos)) return;

		ImDrawList* dl = ImGui::GetBackgroundDrawList();
		float sx = screenPos.x, sy = screenPos.y;

		// Small dot
		dl->AddCircleFilled({ sx, sy }, 4.f, ImColor(255, 200, 0, 255), 12);
		dl->AddCircle({ sx, sy }, 4.f, ImColor(0, 0, 0, 200), 12, 1.f);

		// "C4" label in yellow
		Gui.StrokeText("C4", Vec2(sx, sy - 18.f), ImColor(255, 200, 0, 255), 14.f, true);

		float textY = sy + 8.f;

		// Timer (only when planted and ticking)
		if (bomb.isPlanted && !bomb.isDefused && bomb.blowTime > 0) {
			char timerBuf[32];
			snprintf(timerBuf, sizeof(timerBuf), "%.1fs", bomb.blowTime);

			// Color: green > 10s, yellow 5~10s, red < 5s
			ImColor timerCol;
			if (bomb.blowTime > 10.f)
				timerCol = ImColor(0, 220, 100, 255);
			else if (bomb.blowTime > 5.f)
				timerCol = ImColor(255, 200, 0, 255);
			else
				timerCol = ImColor(255, 50, 50, 255);

			Gui.StrokeText(timerBuf, Vec2(sx, textY), timerCol, 14.f, true);
			textY += 16.f;

			// Defuse info
			if (bomb.isDefusing) {
				// Kit detection: with kit = 5s, without = 10s
				// If defuseTime was initially <= ~5.5s, defuser has kit
				bool hasKit = (bomb.defuseTime <= 5.5f);
				char defBuf[64];
				if (hasKit)
					snprintf(defBuf, sizeof(defBuf), "DEFUSE %.1fs [KIT]", bomb.defuseTime);
				else
					snprintf(defBuf, sizeof(defBuf), "DEFUSE %.1fs", bomb.defuseTime);

				// Green if can make it, red if can't
				bool canDefuse = bomb.blowTime > bomb.defuseTime;
				ImColor defCol = canDefuse ? ImColor(0, 220, 100, 255) : ImColor(255, 50, 50, 255);
				Gui.StrokeText(defBuf, Vec2(sx, textY), defCol, 14.f, true);
			}
		}

		if (bomb.isDefused) {
			Gui.StrokeText("DEFUSED", Vec2(sx, textY), ImColor(0, 220, 100, 255), 14.f, true);
		}
	}

	// ================================================================
	//  Grenade Projectile ESP
	//  Color-coded dots + name + remaining duration
	// ================================================================

	void DrawProjectileESP(const std::vector<GrenadeProjectile>& projectiles,
	                        const float matrix[4][4], const Vec3& localPos)
	{
		if (!IsValidFloat(localPos.x) || !IsValidFloat(localPos.y)) return;
		ImDrawList* dl = ImGui::GetBackgroundDrawList();
		ImColor outline(0, 0, 0, 180);

		for (const auto& proj : projectiles) {
			if (!IsValidFloat(proj.Position.x) || !IsValidFloat(proj.Position.y) || !IsValidFloat(proj.Position.z))
				continue;
			Vec2 screenPos;
			if (!CView::WorldToScreen(matrix, proj.Position, screenPos))
				continue;

			// Per-type color and label
			ImColor dotCol, rangeCol;
			const char* label = "";
			float maxDuration = 0.f;
			switch (proj.Type) {
				case PROJ_SMOKE:  dotCol = ImColor(100, 180, 255, 255); rangeCol = dotCol; label = "Smoke"; maxDuration = 18.0f; break;
				case PROJ_FLASH:  dotCol = ImColor(255, 255, 80, 255);  rangeCol = dotCol; label = "Flash"; break;
				case PROJ_HE:     dotCol = ImColor(255, 60, 60, 255);   rangeCol = ImColor(255, 60, 60, 200); label = "HE"; break;
				case PROJ_MOLOTOV:dotCol = ImColor(255, 140, 0, 255);   rangeCol = ImColor(255, 140, 0, 200); label = "Fire"; maxDuration = 7.0f; break;
				case PROJ_DECOY:  dotCol = ImColor(180, 180, 180, 255); rangeCol = dotCol; label = "Decoy"; break;
				default: continue;
			}

			// Colored dot
			dl->AddCircleFilled({ screenPos.x, screenPos.y }, 5.f, dotCol, 12);
			dl->AddCircle({ screenPos.x, screenPos.y }, 5.f, outline, 12, 1.2f);

			// Label + optional timer
			char text[32];
			bool showTimer = maxDuration > 0.f && (proj.StationaryTimer > 0.5f || !proj.Alive);
			if (showTimer) {
				float remaining = maxDuration - proj.StationaryTimer - proj.DisappearTimer;
				if (remaining < 0.f) remaining = 0.f;
				snprintf(text, sizeof(text), "%s %.1fs", label, remaining);
			} else {
				snprintf(text, sizeof(text), "%s", label);
			}
			Gui.StrokeText(text, Vec2(screenPos.x, screenPos.y - 18.f), dotCol, 13.f, true);

			// Range circle outline for HE, Molotov
			bool showRange = (proj.Type == PROJ_HE || proj.Type == PROJ_MOLOTOV);
			if (showRange && proj.EffectRadius > 0.f) {
				constexpr int CIRCLE_SEGMENTS = 48;
				Vec2 prevScreen;
				bool prevValid = false;

				for (int i = 0; i <= CIRCLE_SEGMENTS; i++) {
					float angle = (float)i / CIRCLE_SEGMENTS * 2.f * 3.14159265f;
					Vec3 worldPt;
					worldPt.x = proj.Position.x + cosf(angle) * proj.EffectRadius;
					worldPt.y = proj.Position.y + sinf(angle) * proj.EffectRadius;
					worldPt.z = proj.Position.z;

					Vec2 sp;
					bool valid = CView::WorldToScreen(matrix, worldPt, sp);
					if (valid && prevValid)
						dl->AddLine({ prevScreen.x, prevScreen.y }, { sp.x, sp.y }, rangeCol, 2.0f);
					prevScreen = sp;
					prevValid = valid;
				}
			}
		}
	}

}

