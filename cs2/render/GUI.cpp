#include "GUI.h"
#include "Render.h"
#include "GrenadeHelper.h"
#include "../config/ConfigMenu.h"
#include "../config/ConfigSaver.h"
#include <shellapi.h>

// ============================================================================
// Color Scheme: Dark Purple-Blue Accent
// Background: #141419 | Card: #1a1a24 | Accent: #7c5cfc | Active: #9b7dff
// ============================================================================

static void setStyles() {
	ImGuiStyle* style = &ImGui::GetStyle();

	style->WindowPadding = ImVec2(8, 8);
	style->WindowRounding = 6.0f;
	style->FramePadding = ImVec2(6, 4);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(8, 6);
	style->ItemInnerSpacing = ImVec2(6, 4);
	style->IndentSpacing = 16.0f;
	style->ScrollbarSize = 12.0f;
	style->ScrollbarRounding = 6.0f;
	style->GrabMinSize = 6.0f;
	style->GrabRounding = 3.0f;
	style->TabRounding = 4.0f;
	style->ChildRounding = 4.0f;
	style->PopupRounding = 4.0f;

	// Main background
	style->Colors[ImGuiCol_WindowBg]           = ImVec4(0.078f, 0.078f, 0.098f, 1.00f);
	style->Colors[ImGuiCol_ChildBg]            = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style->Colors[ImGuiCol_PopupBg]            = ImVec4(0.09f, 0.09f, 0.11f, 0.98f);
	style->Colors[ImGuiCol_Border]             = ImVec4(0.20f, 0.20f, 0.28f, 0.50f);
	style->Colors[ImGuiCol_BorderShadow]       = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

	// Text
	style->Colors[ImGuiCol_Text]               = ImVec4(0.88f, 0.88f, 0.92f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled]        = ImVec4(0.40f, 0.40f, 0.48f, 1.00f);

	// Frame (inputs, combos)
	style->Colors[ImGuiCol_FrameBg]            = ImVec4(0.11f, 0.11f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.16f, 0.16f, 0.22f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive]      = ImVec4(0.20f, 0.20f, 0.28f, 1.00f);

	// Title
	style->Colors[ImGuiCol_TitleBg]            = ImVec4(0.078f, 0.078f, 0.098f, 1.00f);
	style->Colors[ImGuiCol_TitleBgActive]      = ImVec4(0.078f, 0.078f, 0.098f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed]   = ImVec4(0.078f, 0.078f, 0.098f, 0.75f);

	// Button - Accent purple
	style->Colors[ImGuiCol_Button]             = ImVec4(0.486f, 0.361f, 0.988f, 0.60f);
	style->Colors[ImGuiCol_ButtonHovered]      = ImVec4(0.608f, 0.490f, 1.00f, 0.80f);
	style->Colors[ImGuiCol_ButtonActive]       = ImVec4(0.486f, 0.361f, 0.988f, 1.00f);

	// Header (collapsing headers)
	style->Colors[ImGuiCol_Header]             = ImVec4(0.14f, 0.14f, 0.19f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered]      = ImVec4(0.486f, 0.361f, 0.988f, 0.40f);
	style->Colors[ImGuiCol_HeaderActive]       = ImVec4(0.486f, 0.361f, 0.988f, 0.60f);

	// Tabs
	style->Colors[ImGuiCol_Tab]                = ImVec4(0.11f, 0.11f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_TabHovered]         = ImVec4(0.486f, 0.361f, 0.988f, 0.60f);
	style->Colors[ImGuiCol_TabActive]          = ImVec4(0.486f, 0.361f, 0.988f, 0.80f);

	// Scrollbar
	style->Colors[ImGuiCol_ScrollbarBg]        = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab]      = ImVec4(0.30f, 0.30f, 0.38f, 0.60f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.50f, 0.80f);
	style->Colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.486f, 0.361f, 0.988f, 1.00f);

	// Checkbox / Slider
	style->Colors[ImGuiCol_CheckMark]          = ImVec4(0.608f, 0.490f, 1.00f, 1.00f);
	style->Colors[ImGuiCol_SliderGrab]         = ImVec4(0.486f, 0.361f, 0.988f, 0.80f);
	style->Colors[ImGuiCol_SliderGrabActive]   = ImVec4(0.608f, 0.490f, 1.00f, 1.00f);

	// Separator
	style->Colors[ImGuiCol_Separator]          = ImVec4(0.20f, 0.20f, 0.28f, 0.50f);
	style->Colors[ImGuiCol_SeparatorHovered]   = ImVec4(0.486f, 0.361f, 0.988f, 0.60f);
	style->Colors[ImGuiCol_SeparatorActive]    = ImVec4(0.486f, 0.361f, 0.988f, 1.00f);

	// Resize grip
	style->Colors[ImGuiCol_ResizeGrip]         = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style->Colors[ImGuiCol_ResizeGripHovered]  = ImVec4(0.486f, 0.361f, 0.988f, 0.40f);
	style->Colors[ImGuiCol_ResizeGripActive]   = ImVec4(0.486f, 0.361f, 0.988f, 0.80f);

	// Plot
	style->Colors[ImGuiCol_PlotLines]          = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered]   = ImVec4(0.608f, 0.490f, 1.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram]      = ImVec4(0.486f, 0.361f, 0.988f, 0.80f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.608f, 0.490f, 1.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg]     = ImVec4(0.486f, 0.361f, 0.988f, 0.30f);

	style->Colors[ImGuiCol_MenuBarBg]          = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);

	// Frame pacing handled by DXGI waitable swap chain object (VSync)
}

// ============================================================================
// Helper: Draw a styled nav button (left sidebar)
// ============================================================================
static bool NavButton(const char* label, bool active, float width, float height = 32.0f) {
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	if (active) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.486f, 0.361f, 0.988f, 0.80f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.608f, 0.490f, 1.00f, 0.90f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.486f, 0.361f, 0.988f, 1.00f));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	} else {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.20f, 0.28f, 0.60f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.486f, 0.361f, 0.988f, 0.40f));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.65f, 0.72f, 1.0f));
	}
	bool clicked = ImGui::Button(label, ImVec2(width, height));
	ImGui::PopStyleColor(4);
	ImGui::PopStyleVar();
	return clicked;
}

// ============================================================================
// Helper: Section header with accent line
// ============================================================================
static void SectionHeader(const char* label) {
	ImGui::Spacing();
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	dl->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 3, p.y + 16), ImColor(0.486f, 0.361f, 0.988f, 1.0f), 2.0f);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
	ImGui::TextColored(ImVec4(0.88f, 0.88f, 0.95f, 1.0f), "%s", label);
	ImGui::Spacing();
}

// ============================================================================
// Tab 0: Visuals
// ============================================================================
static void DrawTab_Visuals() {
	if (ImGui::CollapsingHeader(lang.header_playerbox.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		Gui.MyCheckBox(lang.visuals_showbox.c_str(), &MenuConfig::ShowBoxESP);
		if (MenuConfig::ShowBoxESP) {
			ImGui::SameLine(0, 16);
			ImGui::ColorEdit4(lang.visuals_boxcolor.c_str(), reinterpret_cast<float*>(&MenuConfig::BoxColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine(0, 8);
			ImGui::TextDisabled("(%s)", lang.visuals_boxcolor.c_str());
			ImGui::SetNextItemWidth(180);
			ImGui::Combo(lang.visuals_boxtype.c_str(), &MenuConfig::BoxType, lang.visuals_boxtypeselect, IM_ARRAYSIZE(lang.visuals_boxtypeselect));

			ImGui::SetNextItemWidth(150);
			ImGui::SliderFloat((lang.visuals_thickness + "##box").c_str(), &MenuConfig::BoxThickness, 0.5f, 5.0f, "%.1f");
			if (MenuConfig::BoxType != 2) {
				ImGui::SetNextItemWidth(150);
				ImGui::SliderFloat((lang.visuals_rounding + "##box").c_str(), &MenuConfig::BoxRounding, 0.f, 10.f, "%.1f");
			}
			if (MenuConfig::BoxType == 2) {
				ImGui::SetNextItemWidth(150);
				ImGui::SliderFloat(lang.visuals_cornersize.c_str(), &MenuConfig::CornerLength, 0.1f, 0.5f, "%.2f");
			}
			Gui.MyCheckBox((lang.visuals_filled + "##box").c_str(), &MenuConfig::BoxFilled);
			if (MenuConfig::BoxFilled) {
				ImGui::SameLine(0, 16);
				ImGui::SetNextItemWidth(120);
				ImGui::SliderFloat(lang.visuals_fillalpha.c_str(), &MenuConfig::BoxFillAlpha, 0.01f, 0.5f, "%.2f");
			}
		}
	}

	if (ImGui::CollapsingHeader(lang.header_skeleton.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		Gui.MyCheckBox(lang.visuals_showbone.c_str(), &MenuConfig::ShowBoneESP);
		if (MenuConfig::ShowBoneESP) {
			ImGui::SameLine(0, 16);
			ImGui::ColorEdit4(lang.visuals_bonecolor.c_str(), reinterpret_cast<float*>(&MenuConfig::BoneColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine(0, 8);
			ImGui::TextDisabled("(%s)", lang.visuals_bonecolor.c_str());
			ImGui::SetNextItemWidth(150);
			ImGui::SliderFloat((lang.visuals_thickness + "##bone").c_str(), &MenuConfig::BoneThickness, 0.5f, 5.0f, "%.1f");
		}

		Gui.MyCheckBox(lang.visuals_headdot.c_str(), &MenuConfig::ShowHeadDot);
		if (MenuConfig::ShowHeadDot) {
			ImGui::SameLine(0, 16);
			ImGui::ColorEdit4("##headdotcol", reinterpret_cast<float*>(&MenuConfig::HeadDotColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SetNextItemWidth(120);
			ImGui::SliderFloat(lang.visuals_dotsize.c_str(), &MenuConfig::HeadDotSize, 1.f, 8.f, "%.1f");
		}

		Gui.MyCheckBox(lang.visuals_showeyeray.c_str(), &MenuConfig::ShowEyeRay);
		if (MenuConfig::ShowEyeRay) {
			ImGui::SameLine(0, 16);
			ImGui::ColorEdit4(lang.visuals_eyeraycolor.c_str(), reinterpret_cast<float*>(&MenuConfig::EyeRayColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine(0, 8);
			ImGui::TextDisabled("(%s)", lang.visuals_eyeraycolor.c_str());
			ImGui::SetNextItemWidth(150);
			ImGui::SliderFloat((lang.visuals_length + "##ray").c_str(), &MenuConfig::EyeRayLength, 10.f, 200.f, "%.0f");
			ImGui::SetNextItemWidth(150);
			ImGui::SliderFloat((lang.visuals_thickness + "##ray").c_str(), &MenuConfig::EyeRayThickness, 0.5f, 5.0f, "%.1f");
		}
	}

	if (ImGui::CollapsingHeader(lang.header_health.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		Gui.MyCheckBox(lang.visuals_showbar.c_str(), &MenuConfig::ShowHealthBar);
		if (MenuConfig::ShowHealthBar) {
			ImGui::SameLine(0, 16);
			ImGui::SetNextItemWidth(160);
			ImGui::Combo(lang.visuals_barpos.c_str(), &MenuConfig::HealthBarType, lang.visuals_heathbarselect, IM_ARRAYSIZE(lang.visuals_heathbarselect));
			if (MenuConfig::HealthBarType == 0) {
				ImGui::SetNextItemWidth(120);
				ImGui::SliderFloat((lang.visuals_barwidth + "##hp").c_str(), &MenuConfig::HealthBarWidth, 2.f, 10.f, "%.1f");
			}
		}

		Gui.MyCheckBox(lang.visuals_armorbar.c_str(), &MenuConfig::ShowArmorBar);
		if (MenuConfig::ShowArmorBar) {
			ImGui::SameLine(0, 16);
			ImGui::ColorEdit4("##armorcol", reinterpret_cast<float*>(&MenuConfig::ArmorBarColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SetNextItemWidth(120);
			ImGui::Combo((lang.grenade_position + "##armor").c_str(), &MenuConfig::ArmorBarType, lang.armorbar_typeselect, IM_ARRAYSIZE(lang.armorbar_typeselect));
			ImGui::SetNextItemWidth(120);
			ImGui::SliderFloat((lang.visuals_width + "##armor").c_str(), &MenuConfig::ArmorBarWidth, 1.f, 8.f, "%.1f");
		}
	}

	if (ImGui::CollapsingHeader(lang.header_info.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		Gui.MyCheckBox(lang.visuals_name.c_str(), &MenuConfig::ShowPlayerName);
		if (MenuConfig::ShowPlayerName) {
			ImGui::SameLine(0, 16);
			ImGui::ColorEdit4("##namecol", reinterpret_cast<float*>(&MenuConfig::NameColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SetNextItemWidth(120);
			ImGui::SliderFloat((lang.visuals_size + "##name").c_str(), &MenuConfig::NameFontSize, 8.f, 24.f, "%.0f");
		}

		Gui.MyCheckBox(lang.visuals_weaponesp.c_str(), &MenuConfig::ShowWeaponESP);
		if (MenuConfig::ShowWeaponESP) {
			ImGui::SameLine(0, 16);
			ImGui::ColorEdit4("##weaponcol", reinterpret_cast<float*>(&MenuConfig::WeaponColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SetNextItemWidth(120);
			ImGui::SliderFloat((lang.visuals_size + "##weapon").c_str(), &MenuConfig::WeaponFontSize, 8.f, 24.f, "%.0f");
		}

		Gui.MyCheckBox(lang.visuals_distance.c_str(), &MenuConfig::ShowDistance);
		if (MenuConfig::ShowDistance) {
			ImGui::SameLine(0, 16);
			ImGui::ColorEdit4("##distcol", reinterpret_cast<float*>(&MenuConfig::DistanceColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SetNextItemWidth(120);
			ImGui::SliderFloat((lang.visuals_size + "##dist").c_str(), &MenuConfig::DistanceFontSize, 8.f, 24.f, "%.0f");
		}
	}

	if (ImGui::CollapsingHeader(lang.header_snapline.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		Gui.MyCheckBox(lang.visuals_line.c_str(), &MenuConfig::ShowLineToEnemy);
		if (MenuConfig::ShowLineToEnemy) {
			ImGui::SameLine(0, 16);
			ImGui::ColorEdit4(lang.visuals_linecolor.c_str(), reinterpret_cast<float*>(&MenuConfig::LineToEnemyColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine(0, 8);
			ImGui::TextDisabled("(%s)", lang.visuals_linecolor.c_str());
			ImGui::SetNextItemWidth(150);
			ImGui::SliderFloat((lang.visuals_thickness + "##line").c_str(), &MenuConfig::LineToEnemyThickness, 0.5f, 5.0f, "%.1f");
			ImGui::SetNextItemWidth(120);
			ImGui::Combo((lang.visuals_origin + "##line").c_str(), &MenuConfig::LineToEnemyOrigin, lang.snapline_originselect, IM_ARRAYSIZE(lang.snapline_originselect));
		}
	}

	if (ImGui::CollapsingHeader("C4 / Bomb", ImGuiTreeNodeFlags_DefaultOpen)) {
		Gui.MyCheckBox("Bomb ESP", &MenuConfig::ShowBombESP);
		if (MenuConfig::ShowBombESP) {
			ImGui::SameLine(0, 16);
			ImGui::ColorEdit4("##bombplanted", reinterpret_cast<float*>(&MenuConfig::BombPlantedColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine(0, 4);
			ImGui::TextDisabled("Planted");
			ImGui::SameLine(0, 12);
			ImGui::ColorEdit4("##bombdefuse", reinterpret_cast<float*>(&MenuConfig::BombDefusingColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine(0, 4);
			ImGui::TextDisabled("Defusing");

			ImGui::ColorEdit4("##bombcarrier", reinterpret_cast<float*>(&MenuConfig::BombCarrierColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine(0, 4);
			ImGui::TextDisabled("Carrier");
			ImGui::SameLine(0, 12);
			ImGui::ColorEdit4("##bombdropped", reinterpret_cast<float*>(&MenuConfig::BombDroppedColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine(0, 4);
			ImGui::TextDisabled("Dropped");
		}
	}

	if (ImGui::CollapsingHeader(lang.proj_header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		Gui.MyCheckBox(lang.proj_enable.c_str(), &MenuConfig::ShowProjectileESP);
		if (MenuConfig::ShowProjectileESP) {
			Gui.MyCheckBox(lang.proj_range.c_str(), &MenuConfig::ShowProjectileRange);
			if (MenuConfig::ShowProjectileRange) {
				ImGui::SetNextItemWidth(180);
				ImGui::SliderFloat(lang.proj_rangealpha.c_str(), &MenuConfig::ProjectileRangeAlpha, 0.02f, 0.5f, "%.2f");
			}
		}
	}
}

// ============================================================================
// Tab 1: Radar
// ============================================================================
static void DrawTab_Radar() {
	// ---- Web Radar (primary) ----
	SectionHeader("Web Radar");

	Gui.MyCheckBox("Enable Web Radar", &MenuConfig::ShowWebRadar);

	if (MenuConfig::ShowWebRadar) {
		ImGui::Spacing();

		ImGui::SetNextItemWidth(120);
		ImGui::InputInt("Port##webradar", &MenuConfig::WebRadarPort);
		if (MenuConfig::WebRadarPort < 1024) MenuConfig::WebRadarPort = 1024;
		if (MenuConfig::WebRadarPort > 65535) MenuConfig::WebRadarPort = 65535;

		ImGui::SetNextItemWidth(180);
		ImGui::SliderInt("Interval (ms)##webradar", &MenuConfig::WebRadarInterval, 50, 500);

		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f),
			"Open in browser: http://<LAN_IP>:5173");
		ImGui::TextColored(ImVec4(0.5f, 0.7f, 0.9f, 1.0f),
			"WS port: %d", MenuConfig::WebRadarPort);
	}

}

// ============================================================================
// Tab 2: Settings
// ============================================================================
static void DrawTab_Settings() {
	if (ImGui::CollapsingHeader(lang.header_general.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		Gui.MyCheckBox(lang.utilities_teamcheck.c_str(), &MenuConfig::TeamCheck);

		ImGui::Spacing();
		ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
		Gui.MyCheckBox(lang.settings_vsync.c_str(), &MenuConfig::VSync);
		if (!MenuConfig::VSync) {
			ImGui::SetNextItemWidth(200);
			ImGui::SliderInt(lang.settings_maxfps.c_str(), &MenuConfig::MaxFrameRate, 0, 500, MenuConfig::MaxFrameRate == 0 ? lang.settings_unlimited.c_str() : "%d");
			if (MenuConfig::MaxFrameRate < 0) MenuConfig::MaxFrameRate = 0;
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", lang.settings_unlimitedtip.c_str());
		}
	}

	if (ImGui::CollapsingHeader(lang.header_safezone.c_str())) {
		Gui.MyCheckBox(lang.safezone_enable.c_str(), &MenuConfig::SafeZoneEnabled);
		if (MenuConfig::SafeZoneEnabled) {
			ImGui::SetNextItemWidth(180);
			ImGui::SliderFloat(lang.safezone_radius.c_str(), &MenuConfig::SafeZoneRadius, 1.f, 300.f, "%.0f px");
			ImGui::SetNextItemWidth(120);
			ImGui::Combo(lang.safezone_shape.c_str(), &MenuConfig::SafeZoneShape, lang.safezone_shapeselect, IM_ARRAYSIZE(lang.safezone_shapeselect));
		}
	}

	if (ImGui::CollapsingHeader(lang.header_system.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		static bool buffer2 = false;
		if (ImGui::Checkbox(lang.utilities_reloadhack.c_str(), &buffer2)) {
			ProcessMgr.Detach();
			globalVars::gameState.store(AppState::SEARCHING_GAME);
			buffer2 = false;
		}

		ImGui::Spacing();
		if (ImGui::Button(lang.utilities_help.c_str(), ImVec2(160, 28))) {
			ShellExecuteA(nullptr, "open", "https://github.com/chao-shushu/CS2-DMA", nullptr, nullptr, SW_SHOWNORMAL);
			TerminateProcess(GetCurrentProcess(), 0);
		}

		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.70f, 0.20f, 0.20f, 0.80f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.30f, 0.30f, 0.90f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.70f, 0.20f, 0.20f, 1.00f));
		if (ImGui::Button(lang.utilities_closehack.c_str(), ImVec2(160, 28))) {
			TerminateProcess(GetCurrentProcess(), 0);
		}
		ImGui::PopStyleColor(3);
	}
}

// ============================================================================
// Tab 3: Config
// ============================================================================
static void DrawTab_Config() {
	ConfigMenu::RenderConfigMenu();
}

// ============================================================================
// Tab 4: Grenade Helper
// ============================================================================
static void DrawTab_Grenade() {
	// --- Display Settings ---
	if (ImGui::CollapsingHeader(lang.header_display.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		Gui.MyCheckBox(lang.grenade_enable.c_str(), &GrenadeHelper::Enabled);

		if (GrenadeHelper::Enabled) {
			Gui.MyCheckBox(lang.grenade_showname.c_str(), &GrenadeHelper::ShowName);
			ImGui::SameLine(0, 16);
			Gui.MyCheckBox(lang.grenade_showbox.c_str(), &GrenadeHelper::ShowBox);
			ImGui::SameLine(0, 16);
			Gui.MyCheckBox(lang.grenade_showline.c_str(), &GrenadeHelper::ShowLine);

			ImGui::Spacing();
			ImGui::SetNextItemWidth(200);
			ImGui::SliderFloat(lang.grenade_maxdistance.c_str(), &GrenadeHelper::MaxDistance, 100.0f, 1000.0f, "%.0f");
			ImGui::SetNextItemWidth(200);
			ImGui::SliderFloat(lang.grenade_boxsize.c_str(), &GrenadeHelper::BoxSize, 4.0f, 20.0f, "%.0f");
		}

		ImGui::Spacing();
		ImGui::Text("%s: %s", lang.grenade_currentmap.c_str(), GrenadeHelper::CurrentMap.c_str());
		if (GrenadeHelper::CurrentThrows) {
			ImGui::SameLine(0, 16);
			ImGui::Text("%s: %d", lang.grenade_availablethrows.c_str(), (int)GrenadeHelper::CurrentThrows->size());
		} else {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", lang.grenade_nomapdata.c_str());
		}

		// Legend
		ImGui::Spacing();
		ImGui::TextColored(ImColor(255, 255, 0), "%s", lang.grenade_flash.c_str());
		ImGui::SameLine(0, 8);
		ImGui::TextColored(ImColor(128, 128, 128), "%s", lang.grenade_smoke.c_str());
		ImGui::SameLine(0, 8);
		ImGui::TextColored(ImColor(255, 128, 0), "%s", lang.grenade_he.c_str());
		ImGui::SameLine(0, 8);
		ImGui::TextColored(ImColor(255, 64, 0), "%s", lang.grenade_molotov.c_str());
	}

	// --- Recording Settings ---
	if (ImGui::CollapsingHeader(lang.header_recording.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		// Record hotkey
		ImGui::Text("%s:", lang.grenade_record_hotkey.c_str());
		ImGui::SameLine();
		if (GrenadeHelper::IsListeningForKey) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.3f, 0.3f, 0.9f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
			if (ImGui::Button(lang.grenade_pressanykey.c_str(), ImVec2(130, 0))) {
				GrenadeHelper::IsListeningForKey = false;
			}
			ImGui::PopStyleColor(3);
		} else {
			if (ImGui::Button(GrenadeHelper::RecordHotKeyName, ImVec2(130, 0))) {
				GrenadeHelper::IsListeningForKey = true;
			}
		}

		// Key listening logic
		if (GrenadeHelper::IsListeningForKey) {
			for (int vk = 0x08; vk <= 0xFE; vk++) {
				if (vk >= 0x01 && vk <= 0x06) continue;
				if (GetAsyncKeyState(vk) & 0x8000) {
					GrenadeHelper::SetCustomHotKey(vk);
					break;
				}
			}
			if (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) GrenadeHelper::SetCustomHotKey(VK_XBUTTON1);
			else if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) GrenadeHelper::SetCustomHotKey(VK_XBUTTON2);
			else if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) GrenadeHelper::SetCustomHotKey(VK_MBUTTON);
			if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) GrenadeHelper::IsListeningForKey = false;
		}

		ImGui::TextColored(ImVec4(0.78f, 0.78f, 0.4f, 1.0f), "%s", lang.grenade_hotkeytip.c_str());

		ImGui::Spacing();
		ImGui::Checkbox(lang.grenade_autosave.c_str(), &GrenadeHelper::AutoSave);
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", lang.grenade_autosavetip.c_str());

		ImGui::SetNextItemWidth(150);
		ImGui::Combo(lang.grenade_defaulttype.c_str(), &GrenadeHelper::DefaultGrenadeType, lang.grenade_typeselect, IM_ARRAYSIZE(lang.grenade_typeselect));
		ImGui::SetNextItemWidth(150);
		ImGui::Combo(lang.grenade_defaultstyle.c_str(), &GrenadeHelper::DefaultThrowStyle, lang.grenade_styleselect, IM_ARRAYSIZE(lang.grenade_styleselect));
	}

	// --- Pending Throws ---
	if (ImGui::CollapsingHeader(lang.header_pending.c_str())) {
		ImGui::Text("%s: %d", lang.grenade_pending_throws.c_str(), (int)GrenadeHelper::PendingThrows.size());

		if (GrenadeHelper::PendingThrows.empty()) {
			ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", lang.grenade_no_pending.c_str());
		} else {
			static int selectedThrow = -1;
			static char throwName[128] = "";
			static int selectedStyle = 0;
			static int selectedType = 0;

			ImGui::BeginChild("PendingThrowsList", ImVec2(0, 130), true);
			for (int i = 0; i < (int)GrenadeHelper::PendingThrows.size(); i++) {
				const auto& pt = GrenadeHelper::PendingThrows[i];
				ImGui::PushID(i);
				bool isSelected = (selectedThrow == i);
				char label[256];
				sprintf_s(label, "#%d [%s] P:%.0f Y:%.0f (%s)",
					i + 1, GrenadeHelper::GetGrenadeTypeName(pt.Type),
					pt.Pitch, pt.Yaw, pt.Timestamp.c_str());
				if (ImGui::Selectable(label, isSelected)) {
					selectedThrow = i;
					strcpy_s(throwName, pt.Name.c_str());
					selectedStyle = pt.Style;
					selectedType = pt.Type;
				}
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Text("%s: (%.1f, %.1f, %.1f)", lang.grenade_position.c_str(), pt.Position.x, pt.Position.y, pt.Position.z);
					ImGui::Text("%s: P=%.1f Y=%.1f", lang.grenade_angle.c_str(), pt.Pitch, pt.Yaw);
					ImGui::Text("%s: %s", lang.grenade_throw_type.c_str(), GrenadeHelper::GetGrenadeTypeName(pt.Type));
					ImGui::Text("%s: %s", lang.grenade_recorded_at.c_str(), pt.Timestamp.c_str());
					ImGui::EndTooltip();
				}
				ImGui::PopID();
			}
			ImGui::EndChild();

			// Edit selected
			ImGui::Separator();
			ImGui::SetNextItemWidth(200);
			ImGui::InputText(lang.grenade_throw_name.c_str(), throwName, sizeof(throwName));
			ImGui::SetNextItemWidth(150);
			ImGui::Combo(lang.grenade_throw_style.c_str(), &selectedStyle, lang.grenade_styleselect, IM_ARRAYSIZE(lang.grenade_styleselect));
			ImGui::SetNextItemWidth(150);
			ImGui::Combo(lang.grenade_throw_type.c_str(), &selectedType, lang.grenade_typeselect, IM_ARRAYSIZE(lang.grenade_typeselect));

			// Buttons row
			if (ImGui::Button(lang.grenade_name_throw.c_str(), ImVec2(90, 25))) {
				if (selectedThrow >= 0 && strlen(throwName) > 0) {
					GrenadeHelper::NamePendingThrow(selectedThrow, throwName, selectedStyle, selectedType);
					selectedThrow = -1;
					throwName[0] = '\0';
				}
			}
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.70f, 0.20f, 0.20f, 0.80f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.30f, 0.30f, 0.90f));
			if (ImGui::Button(lang.grenade_delete.c_str(), ImVec2(70, 25))) {
				if (selectedThrow >= 0) {
					GrenadeHelper::DeletePendingThrow(selectedThrow);
					if (selectedThrow >= (int)GrenadeHelper::PendingThrows.size())
						selectedThrow = (int)GrenadeHelper::PendingThrows.size() - 1;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(lang.grenade_clear_all.c_str(), ImVec2(70, 25))) {
				GrenadeHelper::ClearPendingThrows();
				selectedThrow = -1;
			}
			ImGui::PopStyleColor(2);

			// Save
			ImGui::Spacing();
			if (ImGui::Button(lang.grenade_save_throws.c_str(), ImVec2(140, 28))) {
				if (!GrenadeHelper::CurrentMap.empty()) {
					GrenadeHelper::SaveToFile(GrenadeHelper::CurrentMap);
				}
			}
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "-> %s.json", GrenadeHelper::CurrentMap.c_str());
		}
	}

	// --- Saved Throws Editor ---
	if (ImGui::CollapsingHeader(lang.header_savededitor.c_str())) {
		if (ImGui::Button(lang.grenade_reloadfiles.c_str(), ImVec2(100, 25))) {
			GrenadeHelper::LoadMapData("GrenadeHelper");
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", lang.grenade_reloadtip.c_str());

		ImGui::Spacing();

		static int selectedMapIndex = 0;
		auto availableMaps = GrenadeHelper::GetAvailableMaps();
		if (!availableMaps.empty()) {
			static std::string mapNamesStr;
			mapNamesStr.clear();
			for (size_t i = 0; i < availableMaps.size(); i++) {
				mapNamesStr += availableMaps[i] + '\0';
			}
			ImGui::SetNextItemWidth(200);
			ImGui::Combo(lang.grenade_selectmap.c_str(), &selectedMapIndex, mapNamesStr.c_str(), (int)availableMaps.size());

			std::string selectedMap = availableMaps[selectedMapIndex];

			if (GrenadeHelper::MapsData.find(selectedMap) != GrenadeHelper::MapsData.end()) {
				auto& throws = GrenadeHelper::MapsData[selectedMap].Throws;
				ImGui::Text("%s: %d", lang.grenade_totalthrows.c_str(), (int)throws.size());

				static int selectedSavedThrow = -1;
				static char editName[128] = "";
				static int editStyle = 0;
				static int editType = 0;
				static float editPosX = 0, editPosY = 0, editPosZ = 0;
				static float editPitch = 0, editYaw = 0;

				ImGui::BeginChild("SavedThrowsList", ImVec2(0, 150), true);
				for (int i = 0; i < (int)throws.size(); i++) {
					const auto& t = throws[i];
					ImGui::PushID(i);
					ImColor typeColor = GrenadeHelper::GetGrenadeColor(t.Type);
					bool isSelected = (selectedSavedThrow == i);
					char label[256];
					sprintf_s(label, "#%d [%s] %s", i + 1, GrenadeHelper::GetGrenadeTypeName(t.Type), t.Name.c_str());
					ImGui::PushStyleColor(ImGuiCol_Text, typeColor.Value);
					if (ImGui::Selectable(label, isSelected)) {
						selectedSavedThrow = i;
						strcpy_s(editName, t.Name.c_str());
						editStyle = t.Style;
						editType = t.Type;
						editPosX = t.Position.x;
						editPosY = t.Position.y;
						editPosZ = t.Position.z;
						editPitch = t.Pitch;
						editYaw = t.Yaw;
					}
					ImGui::PopStyleColor();
					if (ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();
						ImGui::Text("%s: %s", lang.grenade_name_label.c_str(), t.Name.c_str());
						ImGui::Text("%s: %s", lang.grenade_throw_style.c_str(), GrenadeHelper::GetStyleName(t.Style));
						ImGui::Text("%s: (%.1f, %.1f, %.1f)", lang.grenade_position.c_str(), t.Position.x, t.Position.y, t.Position.z);
						ImGui::Text("%s: P=%.1f Y=%.1f", lang.grenade_angle.c_str(), t.Pitch, t.Yaw);
						ImGui::EndTooltip();
					}
					ImGui::PopID();
				}
				ImGui::EndChild();

				// Edit form
				if (selectedSavedThrow >= 0 && selectedSavedThrow < (int)throws.size()) {
					ImGui::Separator();
					ImGui::TextColored(ImVec4(0.4f, 0.78f, 1.0f, 1.0f), "%s #%d", lang.grenade_editthrow.c_str(), selectedSavedThrow + 1);

					ImGui::SetNextItemWidth(200);
					ImGui::InputText((lang.grenade_name_label + "##saved").c_str(), editName, sizeof(editName));
					ImGui::SetNextItemWidth(150);
					ImGui::Combo((lang.grenade_throw_style + "##saved").c_str(), &editStyle, lang.grenade_styleselect, IM_ARRAYSIZE(lang.grenade_styleselect));
					ImGui::SetNextItemWidth(150);
					ImGui::Combo((lang.grenade_throw_type + "##saved").c_str(), &editType, lang.grenade_typeselect, IM_ARRAYSIZE(lang.grenade_typeselect));

					// Position on one line
					ImGui::Text("%s:", lang.grenade_position.c_str());
					ImGui::SameLine();
					ImGui::SetNextItemWidth(80);
					ImGui::InputFloat(u8"X##pos", &editPosX, 0.0f, 0.0f, "%.1f");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(80);
					ImGui::InputFloat(u8"Y##pos", &editPosY, 0.0f, 0.0f, "%.1f");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(80);
					ImGui::InputFloat(u8"Z##pos", &editPosZ, 0.0f, 0.0f, "%.1f");

					ImGui::Text("%s:", lang.grenade_angle.c_str());
					ImGui::SameLine();
					ImGui::SetNextItemWidth(80);
					ImGui::InputFloat(u8"Pitch##saved", &editPitch, 0.0f, 0.0f, "%.1f");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(80);
					ImGui::InputFloat(u8"Yaw##saved", &editYaw, 0.0f, 0.0f, "%.1f");

					ImGui::Spacing();
					if (ImGui::Button(lang.grenade_update.c_str(), ImVec2(80, 25))) {
						GrenadeHelper::UpdateThrow(selectedMap, selectedSavedThrow, editName, editStyle, editType,
							editPosX, editPosY, editPosZ, editPitch, editYaw);
					}
					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.70f, 0.20f, 0.20f, 0.80f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.30f, 0.30f, 0.90f));
					if (ImGui::Button((lang.grenade_delete + "##saved").c_str(), ImVec2(80, 25))) {
						GrenadeHelper::DeleteThrow(selectedMap, selectedSavedThrow);
						if (selectedSavedThrow >= (int)throws.size())
							selectedSavedThrow = (int)throws.size() - 1;
					}
					ImGui::PopStyleColor(2);
				}
			}
		} else {
			ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", lang.grenade_nomaps.c_str());
		}
	}
}

// ============================================================================
// Main Menu
// ============================================================================
void Cheats::Menu()
{
	static int active_tab = 0;
	static bool IsMenuInit = false;
	static bool MenuCollapsed = false;
	static ImVec2 CollapsedPos = ImVec2(100, 100);

	if (!IsMenuInit) {
		setStyles();
		IsMenuInit = true;
	}

	// --- Collapsed mode: small draggable square ---
	if (MenuCollapsed) {
		ImGui::SetNextWindowPos(CollapsedPos, ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(36, 36));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.486f, 0.361f, 0.988f, 0.90f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.608f, 0.490f, 1.00f, 0.80f));
		ImGui::Begin("##CollapsedMenu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
		CollapsedPos = ImGui::GetWindowPos();
		ImVec2 wp = ImGui::GetWindowPos();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->AddText(ImVec2(wp.x + 10, wp.y + 8), ImColor(1.0f, 1.0f, 1.0f, 1.0f), "C2");
		if (ImGui::IsWindowHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			MenuCollapsed = false;
		}
		ImGui::End();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(2);
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(globalVars::windowx, globalVars::windowy), ImGuiCond_Once);
	ImGui::Begin("CS2 DMA", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar);
	ImGui::SetWindowSize(ImVec2(globalVars::windowx, globalVars::windowy));
	{
		// ========== Title bar area ==========
		ImVec2 winPos = ImGui::GetWindowPos();
		ImDrawList* dl = ImGui::GetWindowDrawList();

		// Accent line at top (also acts as double-click zone to collapse)
		ImVec2 titleBarMin = ImVec2(winPos.x, winPos.y);
		ImVec2 titleBarMax = ImVec2(winPos.x + globalVars::windowx, winPos.y + 28);
		dl->AddRectFilled(
			ImVec2(winPos.x, winPos.y),
			ImVec2(winPos.x + globalVars::windowx, winPos.y + 3),
			ImColor(0.486f, 0.361f, 0.988f, 1.0f)
		);

		// Double-click title bar to collapse
		if (ImGui::IsMouseHoveringRect(titleBarMin, titleBarMax) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			CollapsedPos = winPos;
			MenuCollapsed = true;
		}

		// Title text
		ImGui::SetCursorPos(ImVec2(12, 8));
		ImGui::TextColored(ImVec4(0.608f, 0.490f, 1.00f, 1.0f), "CS2 / DMA");
		ImGui::SameLine(globalVars::windowx - 220);
		ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.5f, 1.0f), "by github");
		ImGui::SameLine(0, 4);
		ImGui::TextColored(ImVec4(0.608f, 0.490f, 1.00f, 1.0f), "chao-shushu");

		ImGui::SetCursorPosY(28);
		ImGui::Separator();
		ImGui::Spacing();

		float navWidth = 150.0f;
		float contentWidth = globalVars::windowx - navWidth - 24;
		float contentHeight = globalVars::windowy - 50;

		// ========== Left Navigation Panel ==========
		ImGui::BeginChild("##NavPanel", ImVec2(navWidth, contentHeight), false);
		{
			float btnW = navWidth - 8;
			ImGui::Spacing();

			if (NavButton(lang.tab_visuals.c_str(), active_tab == 0, btnW)) active_tab = 0;
			ImGui::Spacing();
			if (NavButton(lang.tab_radar.c_str(), active_tab == 1, btnW)) active_tab = 1;
			ImGui::Spacing();
			if (NavButton(lang.tab_settings.c_str(), active_tab == 2, btnW)) active_tab = 2;
			ImGui::Spacing();
			if (NavButton(lang.tab_config.c_str(), active_tab == 3, btnW)) active_tab = 3;
			ImGui::Spacing();
			if (NavButton(lang.tab_grenade.c_str(), active_tab == 4, btnW)) active_tab = 4;

			// Language selector at bottom
			float remainHeight = ImGui::GetContentRegionAvail().y;
			if (remainHeight > 40) {
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + remainHeight - 36);
			}
			ImGui::Separator();
			ImGui::SetNextItemWidth(btnW);
			if (ImGui::Combo("##lang", &MenuConfig::SelectedLanguage, lang.utilities_langselect, IM_ARRAYSIZE(lang.utilities_langselect))) {
				switch (MenuConfig::SelectedLanguage) {
				case 0: lang.english(); break;
				case 1: lang.chineese(); break;
				default: break;
				}
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// ========== Vertical separator ==========
		{
			ImVec2 cp = ImGui::GetCursorScreenPos();
			dl->AddRectFilled(ImVec2(cp.x - 4, cp.y), ImVec2(cp.x - 2, cp.y + contentHeight), ImColor(0.20f, 0.20f, 0.28f, 0.50f));
		}

		// ========== Right Content Panel ==========
		ImGui::BeginChild("##ContentPanel", ImVec2(contentWidth, contentHeight), false);
		{
			switch (active_tab) {
			case 0: DrawTab_Visuals(); break;
			case 1: DrawTab_Radar(); break;
			case 2: DrawTab_Settings(); break;
			case 3: DrawTab_Config(); break;
			case 4: DrawTab_Grenade(); break;
			default: break;
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}
